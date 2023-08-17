#include "PointLight_System.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <map>

namespace Divine
{
    PointLightSystem::PointLightSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : r_Device{device}
    {
        CreatePipelineLayout(globalSetLayout);
        CreatePipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem()
    {
        vkDestroyPipelineLayout(r_Device.GetDevice(), m_PipelineLayout, nullptr);
    }

    void PointLightSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {globalSetLayout};

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        layoutInfo.pSetLayouts = descriptorSetLayouts.data();
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(r_Device.GetDevice(), &layoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout!");
    }

    void PointLightSystem::CreatePipeline(VkRenderPass renderPass)
    {
        assert(m_PipelineLayout != VK_NULL_HANDLE &&
               "Can't create pipeline without pipeline layout");
        PipelineConfigInfo configInfo{};
        Pipeline::DefaultPipelineConfigInfo(configInfo);
        Pipeline::EnableAlphaBlending(configInfo);
        configInfo.renderPass = renderPass;
        configInfo.pipelineLayout = m_PipelineLayout;

        configInfo.bindingDescriptions.clear();
        configInfo.attributeDescriptions.clear();

        up_Pipeline = std::make_unique<Pipeline>(
            r_Device,
            HOME_DIR "res/shaders/point_light.vert.spv",
            HOME_DIR "res/shaders/point_light.frag.spv",
            configInfo);
    }

    void PointLightSystem::Update(FrameInfo &frameInfo, GlobalUBO &ubo)
    {
        auto rotateLight = glm::rotate(glm::mat4(1.0f),
                                       frameInfo.frameTime,
                                       {0.f, -1.f, 0.f});

        int lightIndex = 0;
        for (auto &kv : frameInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.up_PointLight == nullptr)
                continue;

            assert(lightIndex < MAX_LIGHTS &&
                   "Point lights exceed maximum specified");

            // update light position
            obj.m_ModelMatrix.translation = glm::vec3(rotateLight * glm::vec4(obj.m_ModelMatrix.translation, 1.0f));

            // copy light to ubo
            ubo.PointLights[lightIndex].Position = glm::vec4(obj.m_ModelMatrix.translation, 1.0f);         // w will be ignored
            ubo.PointLights[lightIndex].Color = glm::vec4(obj.m_Color, obj.up_PointLight->lightIntensity); // w is intensity

            ++lightIndex;
        }

        ubo.numLights = lightIndex;
    }

    void PointLightSystem::Render(FrameInfo &frameInfo)
    {
        // sort lights
        std::map<float, DivineGameObject::id_t> sorted; // by default compare by "less" of the "key"
        for (auto &kv : frameInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.up_PointLight == nullptr)
                continue;

            // calculate distence
            auto offset = frameInfo.camera.GetPosition() - obj.m_ModelMatrix.translation;
            float disSquared = glm::dot(offset, offset);
            sorted[disSquared] = obj.GetID();
        }

        up_Pipeline->Bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        // iterate in reverse order so that we render objects from back to front
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            // using ID to find game object since game object is NOT copyable
            auto &obj = frameInfo.gameObjects.at(it->second);

            PointLightPushData push{};
            push.position = glm::vec4(obj.m_ModelMatrix.translation, 1.0f);
            push.color = glm::vec4(obj.m_Color, obj.up_PointLight->lightIntensity);
            push.radius = obj.m_ModelMatrix.scale.x;

            vkCmdPushConstants(frameInfo.commandBuffer,
                               m_PipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               static_cast<uint32_t>(sizeof(PointLightPushData)),
                               &push);

            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }

}