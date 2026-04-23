#define VMA_IMPLEMENTATION

#include "VulkanAllocator.h"

#include "Engine.h"
#include "RenderingDevice.h"

namespace Petal {
    VulkanAllocator::VulkanAllocator(
        const Engine &engine,
        const Renderer &renderer,
        Result &outResult
    ) {
        m_logger = engine.GetLoggerSystem().GetLogger(LoggerSystem::RENDERING_LOGGER);

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

    VmaAllocator VulkanAllocator::GetAllocator() const {
        return m_allocator;
    }

    Result VulkanAllocator::CreateAllocator(
        const Engine &engine,
        const Renderer &renderer
    ) {
        VmaAllocatorCreateInfo createInfo = {};
        createInfo.vulkanApiVersion = renderer.GetRenderingSystem().GetAPIVersion().ToVulkanVersion();
        createInfo.device = renderer.GetDevice()->GetDevice();
        createInfo.physicalDevice = renderer.GetDevice()->GetPhysicalDevice();
        createInfo.instance = renderer.GetRenderingSystem().GetInstance();

        VkResult res = vmaCreateAllocator(&createInfo, &m_allocator);
        PETAL_CHECK_COND(
            res != VK_SUCCESS,
            Result::VULKAN_VMA_CREATION_FAILED,
            m_logger,
            "{}",
            res
        );

        return Result::SUCCESS;
    }
} // Petal
