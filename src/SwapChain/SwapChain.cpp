#include "SwapChain.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <array>

namespace Divine
{
    const unsigned int SwapChain::MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain::SwapChain(Device &device, VkExtent2D windowExtent)
        : r_Device{device}, m_WindowExtent{windowExtent}
    {
        Init();
    }

    SwapChain::SwapChain(Device &device, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous)
        : r_Device{device}, m_WindowExtent{windowExtent}, sp_OldSwapChain{previous}
    {
        Init();

        sp_OldSwapChain = nullptr;
    }

    void SwapChain::Init()
    {
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateDepthResources();
        CreateFrameBuffers();
        CreateSyncObjects();
    }

    SwapChain::~SwapChain()
    {
        for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(r_Device.GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(r_Device.GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(r_Device.GetDevice(), m_InFlightFences[i], nullptr);
        }
        for (auto frameBuffer : m_SwapChainFrameBuffers)
            vkDestroyFramebuffer(r_Device.GetDevice(), frameBuffer, nullptr);
        for (size_t i = 0; i < m_DepthImages.size(); ++i)
        {
            vkDestroyImageView(r_Device.GetDevice(), m_DepthImageViews[i], nullptr);
            vkDestroyImage(r_Device.GetDevice(), m_DepthImages[i], nullptr);
            vkFreeMemory(r_Device.GetDevice(), m_DepthImageMemories[i], nullptr);
        }
        vkDestroyRenderPass(r_Device.GetDevice(), m_RenderPass, nullptr);
        for (auto imageView : m_SwapChainImageViews)
            vkDestroyImageView(r_Device.GetDevice(), imageView, nullptr);
        m_SwapChainImageViews.clear();
        vkDestroySwapchainKHR(r_Device.GetDevice(), m_SwapChain, nullptr);
    }

    void SwapChain::CreateSwapChain()
    {
        SwapChainSupportDetails details = r_Device.GetSwapChainSupportDetails();

        VkExtent2D extent = ChooseSwapChainExtent(details.capabilities);
        VkSurfaceFormatKHR format = ChooseSwapChainSurfaceFormat(details.formats);
        VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(details.presentModes);

        uint32_t imageCount = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
            imageCount = details.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = r_Device.GetSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = format.format;
        createInfo.imageColorSpace = format.colorSpace;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = r_Device.GetQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};
        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = details.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.imageExtent = extent;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = sp_OldSwapChain == nullptr ? VK_NULL_HANDLE : sp_OldSwapChain->GetSwapChain();

        if (vkCreateSwapchainKHR(r_Device.GetDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swap chain!");

        vkGetSwapchainImagesKHR(r_Device.GetDevice(), m_SwapChain, &imageCount, nullptr);
        m_SwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(r_Device.GetDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

        m_SwapChainImageFormat = format.format;
        m_SwapChainImageExtent = extent;
    }

    VkExtent2D SwapChain::ChooseSwapChainExtent(VkSurfaceCapabilitiesKHR capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() ||
            capabilities.currentExtent.height != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;
        else
        {
            VkExtent2D actualExtent = m_WindowExtent;
            actualExtent.width = std::max(
                capabilities.minImageExtent.width,
                std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(
                capabilities.minImageExtent.height,
                std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    VkSurfaceFormatKHR SwapChain::ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats)
    {
        for (VkSurfaceFormatKHR format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return format;
        }

        return formats[0];
    }

    VkPresentModeKHR SwapChain::ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> &presentModes)
    {
        for (VkPresentModeKHR presentMode : presentModes)
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                std::cout << "\tPresent mode: Mailbox" << std::endl;
                return presentMode;
            }
        }

        std::cout << "\tPresent mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    void SwapChain::CreateImageViews()
    {
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        for (size_t i = 0; i < m_SwapChainImageViews.size(); ++i)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_SwapChainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_SwapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(r_Device.GetDevice(), &viewInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create iamge view!");
        }
    }

    void SwapChain::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_SwapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;              /*clear*/
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;            /*store*/
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   /*don't care*/
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; /*don't care*/
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = FindDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;              /*clear*/
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;        /*don't care*/
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   /*don't care*/
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; /*don't care*/
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcAccessMask = VK_ACCESS_NONE;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(r_Device.GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
            throw std::runtime_error("Failed to create render pass!");
    }

    VkFormat SwapChain::FindDepthFormat()
    {
        return r_Device.FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    void SwapChain::CreateDepthResources()
    {
        VkFormat depthFormat = FindDepthFormat();
        m_SwapChainDepthFormat = depthFormat;
        VkExtent2D SwapChainImageExtent = m_SwapChainImageExtent;

        m_DepthImages.resize(m_SwapChainImages.size());
        m_DepthImageMemories.resize(m_SwapChainImages.size());
        m_DepthImageViews.resize(m_SwapChainImages.size());

        for (size_t i = 0; i < m_DepthImages.size(); ++i)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_SwapChainImageExtent.width;
            imageInfo.extent.height = m_SwapChainImageExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

            QueueFamilyIndices indices = r_Device.GetQueueFamilyIndices();
            uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};
            if (indices.graphicsFamily != indices.presentFamily)
            {
                imageInfo.queueFamilyIndexCount = 2;
                imageInfo.pQueueFamilyIndices = queueFamilyIndices;
                imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
            }
            else
            {
                imageInfo.queueFamilyIndexCount = 0;
                imageInfo.pQueueFamilyIndices = nullptr;
                imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            r_Device.CreateImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_DepthImages[i],
                m_DepthImageMemories[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_DepthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(r_Device.GetDevice(), &viewInfo, nullptr, &m_DepthImageViews[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create iamge view!");
        }
    }

    void SwapChain::CreateFrameBuffers()
    {
        m_SwapChainFrameBuffers.resize(m_SwapChainImages.size());

        for (size_t i = 0; i < m_SwapChainFrameBuffers.size(); ++i)
        {
            std::array<VkImageView, 2> attachments = {m_SwapChainImageViews[i], m_DepthImageViews[i]};
            VkExtent2D SwapChainExtent = m_SwapChainImageExtent;

            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferInfo.renderPass = m_RenderPass;
            frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferInfo.pAttachments = attachments.data();
            frameBufferInfo.width = SwapChainExtent.width;
            frameBufferInfo.height = SwapChainExtent.height;
            frameBufferInfo.layers = 1;

            if (vkCreateFramebuffer(r_Device.GetDevice(), &frameBufferInfo, nullptr, &m_SwapChainFrameBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create frame buffer!");
        }
    }

    void SwapChain::CreateSyncObjects()
    {
        m_ImageAvailableSemaphores.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(r_Device.GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) ||
                vkCreateSemaphore(r_Device.GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) ||
                vkCreateFence(r_Device.GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create synchronization objects!");
        }
    }

    VkResult SwapChain::AcquireNextImage(uint32_t *pImageIndex)
    {
        vkWaitForFences(
            r_Device.GetDevice(),
            1,
            &m_InFlightFences[m_CurrentFrame],
            VK_TRUE,
            std::numeric_limits<uint64_t>::max());

        if (vkResetFences(r_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
            throw std::runtime_error("Failed to reset fence!");

        VkResult result = vkAcquireNextImageKHR(
            r_Device.GetDevice(),
            m_SwapChain,
            std::numeric_limits<uint64_t>::max(),
            m_ImageAvailableSemaphores[m_CurrentFrame], // must be a not signaled semaphore
            VK_NULL_HANDLE,
            pImageIndex);

        return result;
    }

    VkResult SwapChain::SubmitCommandBuffers(const VkCommandBuffer *pBuffers, uint32_t *pImageIndex)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = pBuffers;

        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(r_Device.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit command buffer!");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR SwapChains[] = {m_SwapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = SwapChains;

        presentInfo.pImageIndices = pImageIndex;

        VkResult result = vkQueuePresentKHR(r_Device.GetPresentQueue(), &presentInfo);

        m_CurrentFrame = (m_CurrentFrame + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;

        return result;
    }

}