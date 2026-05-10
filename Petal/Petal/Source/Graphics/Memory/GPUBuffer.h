#pragma once

#include "AllocationTracker.h"
#include "Common.h"
#include "IBuffer.h"

namespace Petal {
    class VulkanBuffer;

    class GPUBuffer : public IBuffer {
    public:
        // allocation - where in the backing buffer? use backingBuffer->GetAllocations().Allocate
        GPUBuffer(
            const std::string &name,
            const std::shared_ptr<VulkanBuffer> &backingBuffer,
            Allocation allocation
        );

        // Automatically removes the allocation from the backing buffer's allocation tracker
        ~GPUBuffer();

    public:
        VkDescriptorBufferInfo GetDescriptorInfo() const override;

        const std::string &GetName() const override;

        Allocation GetAllocation() const;

        const std::shared_ptr<VulkanBuffer> &GetBackingBuffer() const;

    private:
        std::string m_name;
        std::shared_ptr<VulkanBuffer> m_backingBuffer;
        Allocation m_allocation;
    };
} // Petal
