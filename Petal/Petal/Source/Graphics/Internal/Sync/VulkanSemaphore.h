#pragma once

#include "Common.h"

namespace Petal {
    class GraphicsContext;

    class VulkanSemaphore {
    public:
        VulkanSemaphore(
            GraphicsContext &renderer,
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
        GraphicsContext &m_renderer;
        VkSemaphore m_handle;
    };
}
