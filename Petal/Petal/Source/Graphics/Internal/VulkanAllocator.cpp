#define VMA_IMPLEMENTATION

#ifndef NDEBUG

// Enable VMA Debug features
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_RECORDING_ENABLED 1

#endif

#include "VulkanAllocator.h"

#include "Engine.h"
#include "RenderingDevice.h"

namespace Petal {
    VulkanAllocator::VulkanAllocator(
        const Engine &engine,
        const GraphicsContext &renderer,
        Result &outResult
    ) {
        m_logger = engine.GetLoggerSystem().GetLogger(LoggerSystem::GRAPHICS_LOGGER);

        outResult = CreateAllocator(
            engine,
            renderer
        );

        if (outResult == Result::SUCCESS) m_logger->Verbose("Created VulkanAllocator");
    }

    VulkanAllocator::~VulkanAllocator() {
        vmaDestroyAllocator(m_allocator);
        m_logger->Verbose("Destroyed VulkanAllocator");
    }

    VmaAllocator VulkanAllocator::GetHandle() const {
        return m_allocator;
    }

    Result VulkanAllocator::CreateAllocator(
        const Engine &engine,
        const GraphicsContext &renderer
    ) {
        VmaAllocatorCreateInfo createInfo = {};
        createInfo.vulkanApiVersion = renderer.GetRenderingSystem().GetAPIVersion().ToVulkanVersion();
        createInfo.device = renderer.GetDevice()->GetHandle();
        createInfo.physicalDevice = renderer.GetDevice()->GetPhysicalDevice();
        createInfo.instance = renderer.GetRenderingSystem().GetInstance();

        VkResult res = vmaCreateAllocator(&createInfo, &m_allocator);
        PETAL_CHECK_COND(
            res != VK_SUCCESS,
            Result::VMA_CREATION_FAILED,
            m_logger,
            "{}",
            res
        );

        return Result::SUCCESS;
    }
} // Petal
