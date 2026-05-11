#include "GPUBuffer.h"

#include "VulkanBuffer.h"

namespace Petal {
    GPUBuffer::GPUBuffer(
        GraphicsContext &context,
        const std::string &name,
        const std::shared_ptr<VulkanBuffer> &backingBuffer,
        Allocation allocation
    ) : m_context(context),
        m_name(name),
        m_backingBuffer(backingBuffer),
        m_allocation(allocation) {
        assert(backingBuffer);
    }

    GPUBuffer::~GPUBuffer() {
        // todo during engine shutdown, this causes a warning to be output since the scheduled function never runs
        std::weak_ptr backingBufferWeak = m_backingBuffer;
        Allocation allocation = m_allocation;

        m_context.GetSwapchain().ScheduleSwapchainFrames([backingBufferWeak, allocation]() {
            std::shared_ptr backingBufferStrong = backingBufferWeak.lock();
            if (!backingBufferStrong)return;
            backingBufferStrong->GetAllocations().Free(allocation);
        });
    }

    VkDescriptorBufferInfo GPUBuffer::GetDescriptorInfo() const {
        return {
            .buffer = m_backingBuffer->GetHandle(),
            .offset = m_allocation.Location,
            .range = m_allocation.Size
        };
    }

    const std::string &GPUBuffer::GetName() const {
        return m_name;
    }

    Allocation GPUBuffer::GetAllocation() const {
        return m_allocation;
    }

    const std::shared_ptr<VulkanBuffer> &GPUBuffer::GetBackingBuffer() const {
        return m_backingBuffer;
    }
} // Petal
