#pragma once

#include "Common.h"

namespace Petal {
    class Renderer;

    class VulkanSemaphore {
    public:
        VulkanSemaphore(
            Renderer &renderer,
            Ref<Logger> logger,
            Result &out
        );

        ~VulkanSemaphore();

        DISABLE_COPY(VulkanSemaphore);

    public:
        VkSemaphore GetHandle() const;

    private:
        Result CreateSemaphore(Ref<Logger> logger);

    private:
        Renderer &m_renderer;
        VkSemaphore m_handle;
    };
}
