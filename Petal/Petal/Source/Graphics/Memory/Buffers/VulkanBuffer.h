#pragma once

#include "../AllocationTracker.h"
#include "Common.h"
#include "BufferType.h"
#include "IBuffer.h"
#include "Graphics/Internal/VulkanAllocator.h"
#include "vulkan/vulkan.h"

namespace Petal {
    struct BufferCreateInfo {
        BufferType BufferType = BufferType::STORAGE_BUFFER;
        bool DeviceLocal = true;
        bool HostVisible = false;
        bool IsTransferDest = true;
        bool IsTransferSource = false;
    };

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

        ~VulkanBuffer();

    public:
        VkBuffer GetHandle() const;

        VmaAllocation GetVMAAllocation() const;

        glm::u32 GetSize() const;

        // All sub allocations inside this buffer
        AllocationTracker &GetAllocations();

        VkDescriptorBufferInfo GetDescriptorInfo() const override;

        const std::string & GetName() const override;

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
