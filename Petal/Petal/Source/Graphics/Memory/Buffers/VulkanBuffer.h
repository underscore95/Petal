#pragma once

#include "BufferCreateInfo.h"
#include "IBuffer.h"
#include "Graphics/Internal/VulkanAllocator.h"
#include "Graphics/Memory/AllocationTracker.h"

namespace Petal {
    class GraphicsContext;

    class VulkanBuffer : public IBuffer {
    public:
        explicit VulkanBuffer(
            GraphicsContext &context,
            const std::shared_ptr<Logger> &logger,
            const std::string &name,
            glm::u32 size,
            const BufferCreateInfo &createInfo,
            Result &resultOut
        );

        ~VulkanBuffer() override;

    public:
        VkBuffer GetHandle() const override;

        VmaAllocation GetVMAAllocation() const;

        glm::u32 GetSize() const override;

        // All sub allocations inside this buffer
        AllocationTracker &GetAllocations();

        VkDescriptorBufferInfo GetDescriptorInfo() const override;

        const std::string &GetName() const override;

    private:
        // Required for IBuffer, but offset is always 0 since a VulkanBuffer is a whole buffer
        glm::u32 GetOffset() const override { return 0; }

    private:
        Result CreateBuffer(std::shared_ptr<Logger> logger);

    private:
        GraphicsContext &m_context;
        std::string m_name;
        glm::u32 m_size;
        BufferCreateInfo m_createInfo;
        VkBuffer m_buffer;
        VmaAllocation m_allocation;
        AllocationTracker m_allocationTracker;
    };
} // Petal
