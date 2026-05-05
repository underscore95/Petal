#pragma once

#include "CommandBuffer.h"
#include "CommandBufferVector.h"

namespace Petal {
    class CommandBufferRef;

    // Holds a strong reference to a command buffer.
    // The command buffer can be stored in CommandBuffer or CommandBufferVector
    class CommandBufferStrongRef {
    public:
        explicit CommandBufferStrongRef(CommandBufferRef weakRef);
        explicit CommandBufferStrongRef(std::shared_ptr<CommandBuffer> commandBuffer);
        CommandBufferStrongRef(std::shared_ptr<CommandBufferVector> commandBufferVector, glm::u32 index);

    public:
        VkCommandBuffer GetHandle() const;

    private:
        std::shared_ptr<CommandBuffer> m_commandBuffer;
        std::shared_ptr<CommandBufferVector> m_commandBufferVector;
        glm::u32 m_vectorIndex;
    };

    // Holds a weak reference to a command buffer.
    // The command buffer can be stored in CommandBuffer or CommandBufferVector
    class CommandBufferRef {
    public:
        explicit CommandBufferRef(std::weak_ptr<CommandBuffer> commandBuffer);

        CommandBufferRef(std::weak_ptr<CommandBufferVector> commandBufferVector, glm::u32 index);

    public:
        explicit operator CommandBufferStrongRef() const;

        std::weak_ptr<CommandBuffer> GetCommandBuffer() const;

        std::weak_ptr<CommandBufferVector> GetCommandBufferVector() const;

        glm::u32 GetVectorIndex() const;

    private:
        std::weak_ptr<CommandBuffer> m_commandBuffer;
        std::weak_ptr<CommandBufferVector> m_commandBufferVector;
        glm::u32 m_vectorIndex;
    };
} // Petal
