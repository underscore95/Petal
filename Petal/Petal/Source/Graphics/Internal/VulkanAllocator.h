#pragma once

#include "Graphics/GraphicsSystem.h"
#include <vk_mem_alloc.h>

namespace Petal {
    class RenderingDevice;

    class VulkanAllocator {
    public:
        VulkanAllocator(
            const Engine &engine,
            const GraphicsContext &renderer,
            Result &outResult
        );

        ~VulkanAllocator();

    public:
        VmaAllocator GetHandle() const;

    private:
        Result CreateAllocator(const Engine &engine, const GraphicsContext &renderer);

    private:
        VmaAllocator m_allocator;
        std::shared_ptr<Logger> m_logger;
    };
} // Petal
