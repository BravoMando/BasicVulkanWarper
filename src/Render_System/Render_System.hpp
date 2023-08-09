#ifndef RenderSystem_HEADER
#define RenderSystem_HEADER

#include "Device.hpp"
#include "Pipeline.hpp"
#include "Game_Object.hpp"
#include "Camera.hpp"

#include <memory>

namespace Divine
{
    class RenderSystem
    {
    public:
        RenderSystem(Device &device, VkRenderPass renderPass);
        ~RenderSystem();
        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void RenderGameObjects(
            VkCommandBuffer commandBuffer,
            std::vector<DivineGameObject> &gameObjects,
            const Camera &camera);

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