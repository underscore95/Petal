#pragma once

#include "Common.h"

namespace Petal {
    class GraphicsContext;

    class VulkanFence {
    public:
        VulkanFence(
            GraphicsContext &renderer,
            VkFenceCreateFlags flags,
            std::shared_ptr<Logger> logger,
            Result &out
        );

        ~VulkanFence();

        DISABLE_COPY(VulkanFence);

    public:
        VkFence GetHandle() const;

    private:
        Result CreateFence(std::shared_ptr<Logger> logger, VkFenceCreateFlags flags);

    private:
        GraphicsContext &m_renderer;
        VkFence m_handle;
    };
} // Petal
