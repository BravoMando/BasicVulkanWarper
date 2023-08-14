#include "PointLight_System.hpp"

#include <iostream>
#include <stdexcept>
#include <array>

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
        // VkPushConstantRange pushConstantRange{};
        // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        // pushConstantRange.offset = 0;
        // pushConstantRange.size = sizeof(PushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {globalSetLayout};

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        layoutInfo.pSetLayouts = descriptorSetLayouts.data();
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(r_Device.GetDevice(), &layoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout!");
    }

    void PointLightSystem::CreatePipeline(VkRenderPass renderPass)
    {
        assert(m_PipelineLayout != VK_NULL_HANDLE &&
               "Can't create pipeline without pipeline layout");
        PipelineConfigInfo configInfo{};
        Pipeline::DefaultPipelineConfigInfo(configInfo);
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

    void PointLightSystem::Render(FrameInfo &frameInfo)
    {
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

        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }

}