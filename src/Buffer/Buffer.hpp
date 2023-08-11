#ifndef BUFFER_HEADER
#define BUFFER_HEADER

#include "Device.hpp"

namespace Divine
{
    class Buffer
    {
    public:
        Buffer(Device &device,
               VkDeviceSize instanceSize,
               uint32_t instanceCount,
               VkBufferUsageFlags usageFlags,
               VkMemoryPropertyFlags memoryPropertyFlags,
               VkDeviceSize minOffsetAlignment = 1);
        ~Buffer();
        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        inline VkBuffer GetBuffer() const { return m_Buffer; }
        inline void *GetMappedMemory() const { return m_Mapped; }
        inline uint32_t GetInstanceCount() const { return m_InstanceCount; }
        inline VkDeviceSize GetInstanceSize() const { return m_InstanceSize; }
        inline VkDeviceSize GetAlignmentSize() const { return m_InstanceSize; } /*?*/
        inline VkBufferUsageFlags GetUsageFlags() const { return m_UsageFlags; }
        inline VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return m_MemoryPropertyFlags; }
        inline VkDeviceSize GetBufferSize() const { return m_BufferSize; }

        VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap();

        void WriteToBuffer(const void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void WriteToIndex(const void *data, int index);
        VkResult FlushIndex(int index);
        VkResult InvalidateIndex(int index);

        VkDescriptorBufferInfo GetDescriptorBufferInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo GetDescriptorBufferInfoForIndex(int index);

    private:
        Device &r_Device;
        VkDeviceSize m_InstanceSize;
        uint32_t m_InstanceCount;
        VkBufferUsageFlags m_UsageFlags;
        VkMemoryPropertyFlags m_MemoryPropertyFlags;
        VkDeviceSize m_BufferSize;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;
        void *m_Mapped = nullptr;
        VkDeviceSize m_AlignmentSize;

        static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);
    };
}

#endif