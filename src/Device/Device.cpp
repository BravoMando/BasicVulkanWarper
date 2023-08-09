#include "Device.hpp"

#include <iostream>
#include <stdexcept>
#include <unordered_set>

namespace Divine
{
    // static member
#ifdef NDEBUG
    const bool Device::s_EnableValidationLayer = false;
#else
    const bool Device::s_EnableValidationLayer = true;
#endif
    const std::vector<const char *> Device::s_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> Device::s_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VKAPI_ATTR VkBool32 VKAPI_CALL Device::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        std::cerr << "[Validation Layer]: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    // proxy functions
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator)
    {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }

    Device::Device(Window &window)
        : r_Window{window}
    {
        CreateInstance();
        if (Device::s_EnableValidationLayer)
            SetUpDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateCommandPool();
    }

    Device::~Device()
    {
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        vkDestroyDevice(m_Device, nullptr);
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        if (Device::s_EnableValidationLayer)
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
    }

    void Device::CreateInstance()
    {
        if (Device::s_EnableValidationLayer && !CheckValidationLayerSupport())
            throw std::runtime_error("Validation layer required but not available!");

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Warper";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (Device::s_EnableValidationLayer)
        {
            VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
            PopulateDebugUtilsMessengerCreateInfo(debugInfo);
            createInfo.enabledLayerCount = static_cast<uint32_t>(Device::s_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = Device::s_ValidationLayers.data();
            createInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&debugInfo);
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instace!");
    }

    bool Device::CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::unordered_set<std::string> requiredLayers(Device::s_ValidationLayers.begin(), Device::s_ValidationLayers.end());
        for (const auto &layer : availableLayers)
            requiredLayers.erase(layer.layerName);

        return requiredLayers.empty();
    }

    std::vector<const char *> Device::GetRequiredExtensions()
    {
        uint32_t glfwExtensionCount;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (Device::s_EnableValidationLayer)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    void Device::PopulateDebugUtilsMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &debugInfo)
    {
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |*/
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugInfo.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(Device::DebugCallback);
        debugInfo.pUserData = nullptr;
    }

    void Device::SetUpDebugMessenger()
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        PopulateDebugUtilsMessengerCreateInfo(debugInfo);
        if (CreateDebugUtilsMessengerEXT(m_Instance, &debugInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug messenger!");
    }

    void Device::CreateSurface()
    {
        if (r_Window.CreateWindowSurface(m_Instance, nullptr, &m_Surface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create surface!");
    }

    void Device::PickPhysicalDevice()
    {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            throw std::runtime_error("Failed to find GPU(s) with Vulkan support!");
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        for (const auto &device : devices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            std::cout << "\tFind GPU: " << properties.deviceName << std::endl;

            if (IsDeviceSuitable(device))
            {
                std::cout << "\tUse GPU: " << properties.deviceName << std::endl;

                m_PhysicalDevice = device;
                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("Failed to find a suitable GPU!");
    }

    bool Device::IsDeviceSuitable(VkPhysicalDevice device)
    {
        auto indice = FindQueueFamilyIndices(device);

        bool extensionSupport = CheckDeviceExtensionSupport(device);

        bool SwapChainAdequate = false;
        if (extensionSupport)
        {
            auto details = QuerySwapChainSupportDetails(device);
            SwapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indice.IsComplete() && extensionSupport && SwapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    QueueFamilyIndices Device::FindQueueFamilyIndices(const VkPhysicalDevice device) const
    {
        QueueFamilyIndices indices;

        uint32_t queueCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> availableQueueFamilies(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, availableQueueFamilies.data());

        uint32_t i = 0;
        for (const auto &queue : availableQueueFamilies)
        {
            if (queue.queueCount > 0 && queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
                indices.graphicsFamilyHasValue = true;
            }
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
                indices.presentFamilyHasValue = true;
            }

            if (indices.IsComplete())
                return indices;
            ++i;
        }

        return indices;
    }

    bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::unordered_set<std::string> requiredExtensions(Device::s_DeviceExtensions.begin(), Device::s_DeviceExtensions.end());
        for (const auto &extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        return requiredExtensions.empty();
    }

    SwapChainSupportDetails Device::QuerySwapChainSupportDetails(const VkPhysicalDevice device) const
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

        uint32_t count = 0;

        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &count, nullptr);
        details.formats.resize(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &count, details.formats.data());

        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &count, nullptr);
        details.presentModes.resize(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &count, details.presentModes.data());

        return details;
    }

    void Device::CreateLogicalDevice()
    {
        QueueFamilyIndices indices = FindQueueFamilyIndices(m_PhysicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::unordered_set<uint32_t> uniqueIndices = {indices.graphicsFamily, indices.presentFamily};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueIndices)
        {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamily;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(Device::s_DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = Device::s_DeviceExtensions.data();

        if (Device::s_EnableValidationLayer)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(Device::s_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = Device::s_ValidationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device!");

        vkGetDeviceQueue(m_Device, indices.graphicsFamily, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, indices.presentFamily, 0, &m_PresentQueue);
    }

    void Device::CreateCommandPool()
    {
        QueueFamilyIndices indices = FindQueueFamilyIndices(m_PhysicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = indices.graphicsFamily;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create command pool!");
    }

    void Device::CreateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer &buffer,
        VkDeviceMemory &bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; /*Exclusive*/

        if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to create command buffer!");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate buffer memory!");

        if (vkBindBufferMemory(m_Device, buffer, bufferMemory, 0) != VK_SUCCESS)
            throw std::runtime_error("Filed to bind buffer memory!");
    }

    uint32_t Device::FindMemoryTypeIndex(uint32_t typeFliter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((typeFliter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }

    VkCommandBuffer Device::BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        if (vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate command buffer!");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin single time command buffer!");

        return commandBuffer;
    }

    void Device::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) /*No Fence*/
            throw std::runtime_error("Failed to submit command buffer!");

        vkQueueWaitIdle(m_GraphicsQueue);

        vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
    }

    void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        EndSingleTimeCommands(commandBuffer);
    }

    void Device::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        EndSingleTimeCommands(commandBuffer);
    }

    void Device::CreateImageWithInfo(
        const VkImageCreateInfo &imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory)
    {
        if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate image memory!");

        if (vkBindImageMemory(m_Device, image, imageMemory, 0) != VK_SUCCESS)
            throw std::runtime_error("Failed to bind image memory!");
    }

    VkFormat Device::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProperties);

            if (tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features)
                return format;
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features)
                return format;
        }

        throw std::runtime_error("Failed to find a supported format!");
    }
}