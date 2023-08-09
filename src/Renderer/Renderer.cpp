#include "Renderer.hpp"

#include <iostream>
#include <stdexcept>
#include <array>

namespace Divine
{
    Renderer::Renderer(Window &window, Device &device)
        : r_Window{window}, r_Device{device}
    {
        RecreateSwapChain();
        CreateCommandBuffers();
    }

    Renderer::~Renderer()
    {
        FreeCommandBuffers();
    }

    void Renderer::RecreateSwapChain()
    {
        auto extent = r_Window.GetExtent();

        while (extent.width == 0 || extent.height == 0)
        {
            extent = r_Window.GetExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(r_Device.GetDevice());

        if (up_SwapChain == nullptr)
            up_SwapChain = std::make_unique<SwapChain>(r_Device, extent);
        else
        {
            std::shared_ptr<SwapChain> oldSwapChain = std::move(up_SwapChain);
            up_SwapChain = std::make_unique<SwapChain>(r_Device, extent, oldSwapChain);

            if (!oldSwapChain->CompareSwapFormats(*up_SwapChain.get()))
                throw std::runtime_error("SwapChain image or depth format has changed!");
        }
    }

    void Renderer::CreateCommandBuffers()
    {
        m_CommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = r_Device.GetCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

        if (vkAllocateCommandBuffers(r_Device.GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate command buffers!");
    }

    void Renderer::FreeCommandBuffers()
    {
        vkFreeCommandBuffers(
            r_Device.GetDevice(),
            r_Device.GetCommandPool(),
            static_cast<uint32_t>(m_CommandBuffers.size()),
            m_CommandBuffers.data());

        m_CommandBuffers.clear();
    }

    VkCommandBuffer Renderer::BeginFrame()
    {
        assert(!m_IsFrameStart &&
               "Can't call BeginFrame while already in progress");

        auto result = up_SwapChain->AcquireNextImage(&m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return VK_NULL_HANDLE;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("Failed to acquire swap chain image!");

        m_IsFrameStart = true;

        auto commandBuffer = GetCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin command buffer!");

        return commandBuffer;
    }

    void Renderer::EndFrame()
    {
        assert(m_IsFrameStart &&
               "Can't call EndFrame when frame not in progress");

        auto commandBuffer = GetCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to end command buffer!");

        auto result = up_SwapChain->SubmitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || r_Window.WasFrameBufferResizd())
        {
            r_Window.ResetFrameBufferResizeFlag();
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to present swap chain image!");

        m_IsFrameStart = false;
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(m_IsFrameStart &&
               "Can't call BeginSwapChainRenderPass when frame not in progress");
        assert(commandBuffer == m_CommandBuffers[m_CurrentFrameIndex] &&
               "Can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = up_SwapChain->GetRenderPass();
        renderPassInfo.framebuffer = up_SwapChain->GetFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = up_SwapChain->GetSwapChainImageExtent();

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(up_SwapChain->GetWidth());
        viewport.height = static_cast<float>(up_SwapChain->GetHeight());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {up_SwapChain->GetWidth(), up_SwapChain->GetHeight()};

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(m_IsFrameStart &&
               "Can't call EndSwapChainRenderPass when frame not in progress");
        assert(commandBuffer == m_CommandBuffers[m_CurrentFrameIndex] &&
               "Can't end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);
    }

}