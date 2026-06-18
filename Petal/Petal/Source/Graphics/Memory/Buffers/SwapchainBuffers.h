#pragma once

#include "Common.h"

namespace Petal {
    class GraphicsContext;
    class VulkanBuffer;
    class IBuffer;

    // Represents N GPUBuffers backed by one VulkanBuffer, where N is the number of swapchain textures
    class SwapchainBuffers {
    public:
        SwapchainBuffers(
            const std::shared_ptr<Logger> &logger,
            GraphicsContext &context,
            const std::vector<std::shared_ptr<IBuffer> > &buffers,
            const std::shared_ptr<VulkanBuffer> &backingBuffer
        );

    public:
        // Call this once per frame
        void Render();

        // Write to all GPUBuffers
        template<typename T>
        void Write(const std::shared_ptr<T> &data) {
            WriteImpl(data, sizeof(T));
        }

        const std::vector<std::shared_ptr<IBuffer> > &GetBuffers() const;

        const std::shared_ptr<VulkanBuffer> &GetBackingBuffer() const;

        const IBuffer &operator[](size_t index) const;

    private:
        void WriteImpl(const std::shared_ptr<void> &data, size_t size);

    private:
        std::shared_ptr<Logger> m_logger;
        GraphicsContext &m_context;
        std::vector<std::shared_ptr<IBuffer> > m_buffers;
        std::shared_ptr<VulkanBuffer> m_backingBuffer;
        std::vector<bool> m_dirtyBuffers;
        std::shared_ptr<void> m_data;
        size_t m_dataSize = 0;
    };
} // Petal
