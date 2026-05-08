#include "CommandBufferRef.h"

namespace Petal {
    CommandBufferStrongRef::CommandBufferStrongRef(
        CommandBufferRef weakRef
    ) : m_commandBuffer(weakRef.GetCommandBuffer().lock()),
        m_commandBufferVector(weakRef.GetCommandBufferVector().lock()),
        m_vectorIndex(weakRef.GetVectorIndex()) {
        assert(m_commandBufferVector && "Attempted to initialize CommandBufferStrongRef with nullptr command buffer");
    }

    CommandBufferStrongRef::CommandBufferStrongRef(
        std::shared_ptr<CommandBuffer> commandBuffer
    ) : m_commandBuffer(commandBuffer) {
        assert(commandBuffer);
    }

    CommandBufferStrongRef::CommandBufferStrongRef(
        std::shared_ptr<CommandBufferVector> commandBufferVector,
        glm::u32 index
    ) : m_commandBufferVector(commandBufferVector),
        m_vectorIndex(index) {
        assert(m_commandBufferVector && "Attempted to initialize CommandBufferStrongRef with nullptr vector");
    }

    VkCommandBuffer CommandBufferStrongRef::GetHandle() const {
        return m_commandBuffer ? m_commandBuffer->GetHandle() : m_commandBufferVector->GetHandle(m_vectorIndex);
    }

    CommandBufferRef::CommandBufferRef(
        std::weak_ptr<CommandBuffer> commandBuffer
    ) : m_commandBuffer(commandBuffer) {
    }

    CommandBufferRef::CommandBufferRef(
        std::weak_ptr<CommandBufferVector> commandBufferVector,
        glm::u32 index
    ) : m_commandBufferVector(commandBufferVector),
        m_vectorIndex(index) {
    }

    CommandBufferRef::operator CommandBufferStrongRef() const {
        return CommandBufferStrongRef(*this);
    }

    std::weak_ptr<CommandBuffer> CommandBufferRef::GetCommandBuffer() const {
        return m_commandBuffer;
    }

    std::weak_ptr<CommandBufferVector> CommandBufferRef::GetCommandBufferVector() const {
        return m_commandBufferVector;
    }

    glm::u32 CommandBufferRef::GetVectorIndex() const {
        return m_vectorIndex;
    }
} // Petal
