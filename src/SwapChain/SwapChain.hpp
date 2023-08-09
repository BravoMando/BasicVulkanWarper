#ifndef SWAPCHAIN_HEADER
#define SWAPCHAIN_HEADER

#include "Device.hpp"

#include <string>
#include <vector>
#include <memory>

namespace Divine
{
    class SwapChain
    {
    public:
        SwapChain(Device &device, VkExtent2D windowExtent);
        SwapChain(Device &device, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
        ~SwapChain();
        SwapChain(const SwapChain &) = delete;
        SwapChain &operator=(const SwapChain &) = delete;

        inline VkSwapchainKHR GetSwapChain() const { return m_SwapChain; }
        inline size_t GetSwapChainImageCount() const { return m_SwapChainImages.size(); }
        inline VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
        inline VkExtent2D GetSwapChainImageExtent() const { return m_SwapChainImageExtent; }
        inline uint32_t GetWidth() const { return m_SwapChainImageExtent.width; }
        inline uint32_t GetHeight() const { return m_SwapChainImageExtent.height; }
        inline VkImageView GetSwapChainImageView(size_t index) const { return m_SwapChainImageViews[index]; }
        inline VkRenderPass GetRenderPass() const { return m_RenderPass; }
        inline float GetExtentAspectRatio() const { return static_cast<float>(m_SwapChainImageExtent.width) / static_cast<float>(m_SwapChainImageExtent.height); }
        inline VkFramebuffer GetFrameBuffer(size_t index) const { return m_SwapChainFrameBuffers[index]; }
        inline bool CompareSwapFormats(const SwapChain &swapChain) const
        {
            return swapChain.m_SwapChainDepthFormat == m_SwapChainDepthFormat && swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat;
        }

        VkFormat FindDepthFormat();

        VkResult AcquireNextImage(uint32_t *pImageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer *pBuffers, uint32_t *pImageIndex);

        static const unsigned int MAX_FRAMES_IN_FLIGHT;

    private:
        void Init();

        void CreateSwapChain();
        VkExtent2D ChooseSwapChainExtent(VkSurfaceCapabilitiesKHR capabilities);
        VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
        VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> &presentModes);

        void CreateImageViews();

        void CreateRenderPass();

        void CreateDepthResources();

        void CreateFrameBuffers();

        void CreateSyncObjects();

    private:
        Device &r_Device;
        VkExtent2D m_WindowExtent;
        std::shared_ptr<SwapChain> sp_OldSwapChain;
        VkSwapchainKHR m_SwapChain;
        std::vector<VkImage> m_SwapChainImages;
        VkFormat m_SwapChainImageFormat;
        VkFormat m_SwapChainDepthFormat;
        VkExtent2D m_SwapChainImageExtent;
        std::vector<VkImageView> m_SwapChainImageViews;
        VkRenderPass m_RenderPass;
        std::vector<VkImage> m_DepthImages;
        std::vector<VkDeviceMemory> m_DepthImageMemories;
        std::vector<VkImageView> m_DepthImageViews;
        std::vector<VkFramebuffer> m_SwapChainFrameBuffers;
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        size_t m_CurrentFrame = 0;
    };
}

#endif