#pragma once

#include "Common.h"

namespace Petal {
    class Renderer;

    class VulkanFence {
    public:
        VulkanFence(
            Renderer &renderer,
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
        Renderer &m_renderer;
        VkFence m_handle;
    };
} // Petal
