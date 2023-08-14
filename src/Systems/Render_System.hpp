#ifndef RENDER_SYSTEM_HEADER
#define RENDER_SYSTEM_HEADER

#include "Device.hpp"
#include "Pipeline.hpp"
#include "Game_Object.hpp"
#include "FrameInfo.hpp"

#include <memory>

namespace Divine
{
    struct PushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    class RenderSystem
    {
    public:
        RenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~RenderSystem();
        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void RenderGameObjects(FrameInfo &frameInfo);

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

    private:
        Device &r_Device;
        VkPipelineLayout m_PipelineLayout;
        std::unique_ptr<Pipeline> up_Pipeline;
    };
}

#endif