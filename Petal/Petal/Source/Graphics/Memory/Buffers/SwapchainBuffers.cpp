#include "SwapchainBuffers.h"

#include "Graphics/Memory/GPUMemorySubsystem.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Internal/VulkanSwapchain.h"

Petal::SwapchainBuffers::SwapchainBuffers(
    const std::shared_ptr<Logger> &logger,
    GraphicsContext &context,
    const std::vector<std::shared_ptr<IBuffer> > &buffers,
    const std::shared_ptr<VulkanBuffer> &backingBuffer
) : m_logger(logger),
    m_context(context),
    m_buffers(buffers),
    m_backingBuffer(backingBuffer),
    m_dirtyBuffers(context.GetSwapchain().NumSwapchainImages()) {
    assert(buffers.size() == context.GetSwapchain().NumSwapchainImages());
}

void Petal::SwapchainBuffers::Render() {
    glm::u32 index = m_context.GetSwapchain().GetSwapchainIndex();
    if (!m_dirtyBuffers[index]) return;
    m_dirtyBuffers[index] = false;

    // todo write async until we reach this index again
    Result result = m_context.GetMemorySubsystem().Write(*(m_buffers[index]), m_data.get(), m_dataSize);
    if (result != Result::SUCCESS) [[unlikely]] {
        m_logger->Warn("Failed to write to MultipleBuffers index {} because {}", index, result);
    }
}

const std::vector<std::shared_ptr<Petal::IBuffer> > &Petal::SwapchainBuffers::GetBuffers() const {
    return m_buffers;
}

const std::shared_ptr<Petal::VulkanBuffer> &Petal::SwapchainBuffers::GetBackingBuffer() const {
    return m_backingBuffer;
}

const Petal::IBuffer &Petal::SwapchainBuffers::operator[](size_t index) const {
    return *m_buffers[index];
}

void Petal::SwapchainBuffers::WriteImpl(
    const std::shared_ptr<void> &data,
    size_t size
) {
    m_data = data;
    m_dataSize = size;
    std::fill(m_dirtyBuffers.begin(), m_dirtyBuffers.end(), true);

    glm::u32 index = m_context.GetSwapchain().GetSwapchainIndex();
    m_dirtyBuffers[index] = false;
    Result result = m_context.GetMemorySubsystem().Write(*(m_buffers[index]), m_data.get(), m_dataSize);
    if (result != Result::SUCCESS) [[unlikely]] {
        m_logger->Warn("Failed to write to MultipleBuffers index {} because {}", index, result);
    }
}
