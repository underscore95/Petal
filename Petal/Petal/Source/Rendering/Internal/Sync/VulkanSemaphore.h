#pragma once

#include "Common.h"

namespace Petal {
    class Renderer;

    class VulkanSemaphore {
    public:
        VulkanSemaphore(
            Renderer &renderer,
            std::shared_ptr<Logger> logger,
            Result &out
        );

        ~VulkanSemaphore();

        DISABLE_COPY(VulkanSemaphore);

    public:
        VkSemaphore GetHandle() const;

    private:
        Result CreateSemaphore(std::shared_ptr<Logger> logger);

    private:
        Renderer &m_renderer;
        VkSemaphore m_handle;
    };
}
