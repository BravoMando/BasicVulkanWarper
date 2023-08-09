#ifndef DEVICE_HEADER
#define DEVICE_HEADER

#include "Window.hpp"

#include <string>
#include <vector>

namespace Divine
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices
    {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;

        inline bool IsComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class Device
    {
    public:
        Device(Window &window);
        ~Device();
        Device(const Window &) = delete;
        Device &operator=(const Device &) = delete;

        inline VkInstance GetInstance() const { return m_Instance; }
        inline VkSurfaceKHR GetSurface() const { return m_Surface; }
        inline VkDevice GetDevice() const { return m_Device; }
        inline VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        inline VkQueue GetPresentQueue() const { return m_PresentQueue; }
        inline VkCommandPool GetCommandPool() const { return m_CommandPool; }

        inline QueueFamilyIndices GetQueueFamilyIndices() const { return FindQueueFamilyIndices(m_PhysicalDevice); }
        inline SwapChainSupportDetails GetSwapChainSupportDetails() const { return QuerySwapChainSupportDetails(m_PhysicalDevice); }

        void CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer &buffer,
            VkDeviceMemory &bufferMemory);
        uint32_t FindMemoryTypeIndex(uint32_t typeFliter, VkMemoryPropertyFlags properties);

        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
        void CreateImageWithInfo(
            const VkImageCreateInfo &imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage &image,
            VkDeviceMemory &imageMemory);

        VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    private:
        void CreateInstance();
        bool CheckValidationLayerSupport();
        std::vector<const char *> GetRequiredExtensions();
        void PopulateDebugUtilsMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &debugInfo);

        void SetUpDebugMessenger();

        void CreateSurface();

        void PickPhysicalDevice();
        bool IsDeviceSuitable(VkPhysicalDevice device);
        QueueFamilyIndices FindQueueFamilyIndices(const VkPhysicalDevice device) const;
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails QuerySwapChainSupportDetails(const VkPhysicalDevice device) const;

        void CreateLogicalDevice();

        void CreateCommandPool();

    private:
        Window &r_Window;
        VkInstance m_Instance;
        VkDebugUtilsMessengerEXT m_DebugMessenger;
        VkSurfaceKHR m_Surface;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkDevice m_Device;
        VkQueue m_GraphicsQueue;
        VkQueue m_PresentQueue;
        VkCommandPool m_CommandPool;

    public:
        static const bool s_EnableValidationLayer;
        static const std::vector<const char *> s_ValidationLayers;
        static const std::vector<const char *> s_DeviceExtensions;
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);
    };
}

#endif