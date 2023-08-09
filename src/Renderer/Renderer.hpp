#ifndef RENDERER_HEADER
#define RENDERER_HEADER

#include "Window.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"

#include <memory>
#include <assert.h>

namespace Divine
{
    class Renderer
    {
    public:
        Renderer(Window &window, Device &device);
        ~Renderer();
        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        inline bool IsFrameInProgress() const { return m_IsFrameStart; }
        inline VkCommandBuffer GetCurrentCommandBuffer() const
        {
            assert(m_IsFrameStart &&
                   "Can't get command buffer when frame not in progress");
            return m_CommandBuffers[m_CurrentFrameIndex];
        }
        inline int GetFrameIndex() const
        {
            assert(m_IsFrameStart &&
                   "Can't get frame index when frame not in progress");
            return m_CurrentFrameIndex;
        }
        inline VkRenderPass GetSwapChainRenderPass() const { return up_SwapChain->GetRenderPass(); }
        inline float GetAspectRatio() const { return up_SwapChain->GetExtentAspectRatio(); }

        VkCommandBuffer BeginFrame();
        void EndFrame();
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void RecreateSwapChain();
        void CreateCommandBuffers();
        void FreeCommandBuffers();

    private:
        Window &r_Window;
        Device &r_Device;
        std::unique_ptr<SwapChain> up_SwapChain;
        std::vector<VkCommandBuffer> m_CommandBuffers;
        uint32_t m_CurrentImageIndex;
        uint32_t m_CurrentFrameIndex = 0;
        bool m_IsFrameStart = false;
    };
}

#endif