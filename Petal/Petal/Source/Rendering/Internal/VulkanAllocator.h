#pragma once

#include "Rendering/RenderingSystem.h"
#include <vk_mem_alloc.h>

namespace Petal {
    class RenderingDevice;

    class VulkanAllocator {
    public:
        VulkanAllocator(
            const Engine &engine,
            const Renderer &renderer,
            Result &outResult
        );

        ~VulkanAllocator();

    public:
        VmaAllocator GetAllocator() const;

    private:
        Result CreateAllocator(const Engine &engine, const Renderer &renderer);

    private:
        VmaAllocator m_allocator;
        Ref<Logger> m_logger;
    };
} // Petal
