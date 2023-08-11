#include "Buffer.hpp"

#include <assert.h>
#include <string.h>

#include <stdexcept>

namespace Divine
{
    VkDeviceSize Buffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
    {
        if (minOffsetAlignment > 0)
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        else
            return instanceSize;
    }

    Buffer::Buffer(Device &device,
                   VkDeviceSize instanceSize,
                   uint32_t instanceCount,
                   VkBufferUsageFlags usageFlags,
                   VkMemoryPropertyFlags memoryPropertyFlags,
                   VkDeviceSize minOffsetAlignment)
        : r_Device{device}, m_InstanceSize{instanceSize}, m_InstanceCount{instanceCount}, m_UsageFlags{usageFlags}, m_MemoryPropertyFlags{memoryPropertyFlags}
    {
        m_AlignmentSize = Buffer::GetAlignment(instanceSize, minOffsetAlignment);
        m_BufferSize = m_AlignmentSize * instanceCount;
        r_Device.CreateBuffer(m_BufferSize, usageFlags, memoryPropertyFlags, m_Buffer, m_Memory);
    }

    Buffer::~Buffer()
    {
        Unmap();
        vkFreeMemory(r_Device.GetDevice(), m_Memory, nullptr);
        vkDestroyBuffer(r_Device.GetDevice(), m_Buffer, nullptr);
    }

    /**
     * Map a memory range of this buffer
     *
     * @param size (Optional) Size of the memory range to map. VK_WHOLE_SIZE by default
     * @param offset (Optional) Byte offset from beginning. 0 by default
     *
     * @return VkResult of the buffer mapping call
     */
    VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
    {
        assert(m_Buffer && m_Memory && "Can't call Map function before buffer creation");

        return vkMapMemory(r_Device.GetDevice(), m_Memory, offset, size, 0, &m_Mapped);
    }

    /**
     *Unmap a memory range of this buffer
     */
    void Buffer::Unmap()
    {
        if (m_Mapped)
        {
            vkUnmapMemory(r_Device.GetDevice(), m_Memory);
            m_Mapped = nullptr;
        }
    }

    /**
     * Copies the specified data to the mapped buffer
     *
     * @param data Pointer to the data to copy from
     * @param size (Optional) Size of the data to copy. VK_WHOLE_SIZE by default
     *
     * @param offset (Optional) Byte offset from beginning of mapped region. 0 by default
     */
    void Buffer::WriteToBuffer(const void *data, VkDeviceSize size, VkDeviceSize offset)
    {
        assert(m_Mapped && "Can't copy to umapped buffer memory");

        if (size == VK_WHOLE_SIZE)
            memcpy(m_Mapped, data, m_BufferSize);
        else
        {
            char *memOffset = reinterpret_cast<char *>(m_Mapped);
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    /**
     * Flush a memory range of the buffer to make it visible to the device
     *
     * @param size (Optional) Size of the memory range to flush. VK_SHOLE_SIZE by default
     * @param offset (Optional) Byte offset from beginning. 0 by default
     *
     * @note Only required for non-coherent memory
     */
    VkResult Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange{};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_Memory;
        mappedRange.offset = offset;
        mappedRange.size = size;

        return vkFlushMappedMemoryRanges(r_Device.GetDevice(), 1, &mappedRange);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @param size (Optional) Size of the memory range to invalidate. VK_WHOLE_SIZE by default
     * @param offset (Optional) Byte offset from beginning. 0 by default
     *
     * @return VkResult of the invalidate call
     *
     * @note Only required for non-coherent memory
     */
    VkResult Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_Memory;
        mappedRange.offset = offset;
        mappedRange.size = size;

        return vkInvalidateMappedMemoryRanges(r_Device.GetDevice(), 1, &mappedRange);
    }

    /**
     * Copies m_InstanceSize bytes of data to the mapped buffer at an offset of index * m_AlignmentSize
     *
     * @param data Pointer to the data to copy from
     * @param index Used in offset calculation
     */
    void Buffer::WriteToIndex(const void *data, int index)
    {
        WriteToBuffer(data, m_InstanceSize, index * m_AlignmentSize);
    }

    /**
     *  Flush the memory range at index * m_AlignmentSize of the buffer to make it visible to the device
     *
     * @param index Used in offset calculation
     *
     */
    VkResult Buffer::FlushIndex(int index)
    {
        return Flush(m_AlignmentSize, index * m_AlignmentSize);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @param index Specifies the region index * m_AlignmentSize to invalidate
     *
     * @note Only required for non-coherent memory
     *
     * @return VkResult of the invalidate call
     */
    VkResult Buffer::InvalidateIndex(int index)
    {
        return Invalidate(m_AlignmentSize, index * m_AlignmentSize);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param size (Optional) Size of the memory range of the descriptor. VK_SHOLE_SIZE by default
     * @param offset (Optional) Byte offset from beginning. 0 by default
     *
     * @return VkDescriptorBufferInfo of specified offset and range
     */
    VkDescriptorBufferInfo Buffer::DescriptorBufferInfo(VkDeviceSize size, VkDeviceSize offset)
    {
        return VkDescriptorBufferInfo{m_Buffer, offset, size};
    }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */
    VkDescriptorBufferInfo Buffer::DescriptorBufferInfoForIndex(int index)
    {
        return DescriptorBufferInfo(m_AlignmentSize, index * m_AlignmentSize);
    }

}