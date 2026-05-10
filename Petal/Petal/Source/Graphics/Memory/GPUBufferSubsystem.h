#pragma once
#include "VulkanBuffer.h"

namespace Petal {
    class GPUBuffer;
    class GraphicsContext;

    class GPUBufferSubsystem {
    public:
        GPUBufferSubsystem(
            GraphicsContext &renderer,
            const std::shared_ptr<Logger> &logger,
            Result &resultOut
        );

        ~GPUBufferSubsystem();

    public:
        // TODO: System for writing to buffers every frame which submits frame commands instead of blocking commands?

        // Create a buffer
        // Independent means that the internal VulkanBuffer is exclusively for this GPUBuffer and is not shared
        AllocatedOptional<GPUBuffer> CreateIndependentBuffer(
            const std::string &name,
            glm::u32 size,
            const BufferCreateInfo &createInfo = {}
        );

        // Create a GPUBuffer which is backed by all or a subsection of a VulkanBuffer
        // size - size of the GPUBuffer
        // offset - where in the backingBuffer this buffer is located
        AllocatedOptional<GPUBuffer> CreateBackedBuffer(
            const std::string &name,
            glm::u32 size,
            std::shared_ptr<VulkanBuffer> backingBuffer
        );

        // Create a Vulkan buffer
        // You must create one (or more) GPUBuffer which use the VulkanBuffer as a backing buffer
        AllocatedOptional<VulkanBuffer> CreateVulkanBuffer(
            const std::string &name,
            glm::u32 size,
            const BufferCreateInfo &createInfo = {}
        );

        // Write to a buffer
        Result Write(
            const GPUBuffer &buffer,
            const void *data,
            glm::u32 size,
            glm::u32 bufferOffset = 0
        );

    private:
        Result CreateTransferBuffer();

        Result CreateCommandBuffer();

    private:
        GraphicsContext &m_renderer;
        std::shared_ptr<Logger> m_logger;
        std::unique_ptr<VulkanBuffer> m_transferBuffer;
        std::unique_ptr<CommandBuffer> m_commandBuffer;
    };
} // Petal
