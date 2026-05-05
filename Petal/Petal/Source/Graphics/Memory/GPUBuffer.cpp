#include "GPUBuffer.h"

#include "VulkanBuffer.h"

namespace Petal {
    GPUBuffer::GPUBuffer(
        const std::string &name,
        std::shared_ptr<VulkanBuffer> backingBuffer,
        glm::u32 size,
        glm::u32 offset
    ) : m_name(name),
        m_backingBuffer(backingBuffer),
        m_size(size),
        m_offset(offset) {
        assert(backingBuffer);
    }

    VkDescriptorBufferInfo GPUBuffer::GetDescriptorInfo() const {
        return {
            .buffer = m_backingBuffer->GetHandle(),
            .offset = m_offset,
            .range = m_size
        };
    }

    std::string GPUBuffer::GetName() const {
        return m_name;
    }

    glm::u32 GPUBuffer::GetSize() const {
        return m_size;
    }

    glm::u32 GPUBuffer::GetOffset() const {
        return m_offset;
    }

    const std::shared_ptr<VulkanBuffer> &GPUBuffer::GetBackingBuffer() const {
        return m_backingBuffer;
    }
} // Petal
