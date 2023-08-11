#include "Render_System.hpp"

#include <iostream>
#include <stdexcept>
#include <array>

namespace Divine
{
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

    void RenderSystem::RenderGameObjects(FrameInfo &frameInfo, std::vector<DivineGameObject> &gameObjects)
    {
        up_Pipeline->Bind(frameInfo.commandBuffer);

        for (auto &obj : gameObjects)
        {
            PushConstantData push{};
            push.normalMatrix = obj.m_ModelMatrix.GetNormalMat();
            push.transform = frameInfo.camera.GetProjectionMat() * frameInfo.camera.GetViewMat() * obj.m_ModelMatrix.GetModelMat(); // MVP matrix

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                static_cast<uint32_t>(sizeof(PushConstantData)),
                &push);

            obj.sp_Model->Bind(frameInfo.commandBuffer);
            obj.sp_Model->Draw(frameInfo.commandBuffer);
        }
    }

}