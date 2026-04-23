#pragma once

#include "Common.h"

namespace Petal {
    class Renderer;

    class VulkanFence {
    public:
        VulkanFence(
            Renderer &renderer,
            VkFenceCreateFlags flags,
            Ref<Logger> logger,
            Result &out
        );

        ~VulkanFence();

        DISABLE_COPY(VulkanFence);

    public:
        VkFence GetHandle() const;

    private:
        Result CreateFence(Ref<Logger> logger, VkFenceCreateFlags flags);

    private:
        Renderer &m_renderer;
        VkFence m_handle;
    };
} // Petal
