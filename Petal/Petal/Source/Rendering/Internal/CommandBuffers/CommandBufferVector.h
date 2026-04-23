#pragma once

#include "Common.h"
#include <vulkan/vulkan.h>

namespace Petal {
    class CommandBufferVector {
    public:
        // Create in renderer
        CommandBufferVector(
            Ref<Logger> logger,
            VkDevice device,
            VkCommandPool pool,
            VkCommandBufferLevel level,
            glm::u32 count,
            Result &out
        );

        ~CommandBufferVector();

        CommandBufferVector(CommandBufferVector&& other) noexcept;
        CommandBufferVector& operator=(CommandBufferVector&& other) noexcept;

    public:
        Result Begin(glm::u32 index, VkCommandBufferUsageFlags usageFlags) const;
        Result End(glm::u32 index) const;

        VkCommandBuffer GetHandle(glm::u32 index) const;

        glm::u32 Size() const;

    private:
        Result CreateCommandBuffers(VkCommandBufferLevel level, glm::u32 count);

    private:
        Ref<Logger> m_logger;
        VkDevice m_device;
        VkCommandPool m_commandPool;
        std::vector<VkCommandBuffer> m_handles;
    };
}