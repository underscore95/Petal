#pragma once

#include "Common.h"

namespace Petal {
    class VulkanBuffer;

    class GPUBuffer {
    public:
        GPUBuffer(
            const std::string &name,
            std::shared_ptr<VulkanBuffer> backingBuffer,
            glm::u32 size,
            glm::u32 offset = 0
        );

    public:
        VkDescriptorBufferInfo GetDescriptorInfo() const;

        std::string GetName() const;

        glm::u32 GetSize() const;

        glm::u32 GetOffset() const;

        const std::shared_ptr<VulkanBuffer> &GetBackingBuffer() const;

    private:
        std::string m_name;
        std::shared_ptr<VulkanBuffer> m_backingBuffer;
        glm::u32 m_size;
        glm::u32 m_offset;
    };
} // Petal
