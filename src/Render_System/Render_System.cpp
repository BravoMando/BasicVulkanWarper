#include "Render_System.hpp"

#include <iostream>
#include <stdexcept>
#include <array>

namespace Divine
{
    struct PushConstantData
    {
        glm::mat4 transform{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    RenderSystem::RenderSystem(Device &device, VkRenderPass renderPass)
        : r_Device{device}
    {
        CreatePipelineLayout();
        CreatePipeline(renderPass);
    }

    RenderSystem::~RenderSystem()
    {
        vkDestroyPipelineLayout(r_Device.GetDevice(), m_PipelineLayout, nullptr);
    }

    void RenderSystem::CreatePipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantData);

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 0;
        layoutInfo.pSetLayouts = nullptr;
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(r_Device.GetDevice(), &layoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout!");
    }

    void RenderSystem::CreatePipeline(VkRenderPass renderPass)
    {
        assert(m_PipelineLayout != VK_NULL_HANDLE &&
               "Can't create pipeline without pipeline layout");
        PipelineConfigInfo configInfo{};
        Pipeline::DefaultPipelineConfigInfo(configInfo);
        configInfo.renderPass = renderPass;
        configInfo.pipelineLayout = m_PipelineLayout;

        up_Pipeline = std::make_unique<Pipeline>(
            r_Device,
            HOME_DIR "res/shaders/basic_vert.spv",
            HOME_DIR "res/shaders/basic_frag.spv",
            configInfo);
    }

    void RenderSystem::RenderGameObjects(
        VkCommandBuffer commandBuffer,
        std::vector<DivineGameObject> &gameObjects,
        const Camera &camera)
    {
        up_Pipeline->Bind(commandBuffer);

        for (auto &obj : gameObjects)
        {
            // obj.m_ModelMatrix.rotation.y = glm::mod(obj.m_ModelMatrix.rotation.y + 0.01f, glm::two_pi<float>());
            // obj.m_ModelMatrix.rotation.x = glm::mod(obj.m_ModelMatrix.rotation.x + 0.005f, glm::two_pi<float>());

            PushConstantData push{};
            push.normalMatrix = obj.m_ModelMatrix.GetNormalMat();
            push.transform = camera.GetProjectionMat() * camera.GetViewMat() * obj.m_ModelMatrix.GetModelMat(); // MVP matrix

            vkCmdPushConstants(
                commandBuffer,
                m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                static_cast<uint32_t>(sizeof(PushConstantData)),
                &push);

            obj.sp_Model->Bind(commandBuffer);
            obj.sp_Model->Draw(commandBuffer);
        }
    }

}