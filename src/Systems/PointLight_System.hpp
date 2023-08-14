#ifndef POINTLIGHT_SYSTEM_HEADER
#define POINTLIGHT_SYSTEM_HEADER

#include "Device.hpp"
#include "Pipeline.hpp"
#include "Game_Object.hpp"
#include "FrameInfo.hpp"

#include <memory>

namespace Divine
{
    // struct PushConstantData
    // {
    //     glm::mat4 modelMatrix{1.0f};
    //     glm::mat4 normalMatrix{1.0f};
    // };

    class PointLightSystem
    {
    public:
        PointLightSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();
        PointLightSystem(const PointLightSystem &) = delete;
        PointLightSystem &operator=(const PointLightSystem &) = delete;

        void Render(FrameInfo &frameInfo);

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