#pragma once

#include "Common.h"
#include <vulkan/vulkan.h>

namespace Petal {
    class GraphicsContext;

    class CommandBufferVector {
    public:
        // Create in renderer
        CommandBufferVector(
            std::shared_ptr<Logger> logger,
            GraphicsContext &context,
            VkCommandPool pool,
            VkCommandBufferLevel level,
            glm::u32 count,
            Result &out
        );

        ~CommandBufferVector();

        CommandBufferVector(CommandBufferVector &&other) noexcept;

    public:
        Result Begin(glm::u32 index, VkCommandBufferUsageFlags usageFlags) const;

        Result End(glm::u32 index) const;

        Result BeginAll(VkCommandBufferUsageFlags usageFlags) const;

        Result EndAll() const;

        VkCommandBuffer GetHandle(glm::u32 index) const;

        glm::u32 Size() const;

        // Check if the size of this vector is equal to the number of swapchain images
        bool IsSwapchainSize() const;

    private:
        Result CreateCommandBuffers(VkCommandBufferLevel level, glm::u32 count);

    private:
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        VkCommandPool m_commandPool;
        std::vector<VkCommandBuffer> m_handles;
    };
}
