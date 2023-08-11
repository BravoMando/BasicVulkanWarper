#ifndef RenderSystem_HEADER
#define RenderSystem_HEADER

#include "Device.hpp"
#include "Pipeline.hpp"
#include "Game_Object.hpp"
#include "FrameInfo.hpp"

#include <memory>

namespace Divine
{
    struct PushConstantData
    {
        glm::mat4 transform{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    class RenderSystem
    {
    public:
        RenderSystem(Device &device, VkRenderPass renderPass);
        ~RenderSystem();
        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void RenderGameObjects(FrameInfo &frameInfo, std::vector<DivineGameObject> &gameObjects);

    private:
        void CreatePipelineLayout();
        void CreatePipeline(VkRenderPass renderPass);

    private:
        Device &r_Device;
        VkPipelineLayout m_PipelineLayout;
        std::unique_ptr<Pipeline> up_Pipeline;
    };
}

#endif