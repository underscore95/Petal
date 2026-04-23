#include "Renderer.h"
#include "RenderingSystem.h"
#include "Engine.h"
#include "Window/Window.h"
#include "Memory/MemorySystem.h"
#include "Internal/VulkanAllocator.h"
#include "Internal/VulkanQueue.h"
#include "Internal/CommandBuffers/CommandBuffer.h"
#include "Internal/CommandBuffers/CommandBufferVector.h"
#include "Internal/RenderingDevice.h"
#include "Internal/VulkanSwapchain.h"
#include "Internal/Sync/VulkanFence.h"
#include "Internal/Sync/VulkanSemaphore.h"

namespace Petal {
    Renderer::Renderer(
        Engine &engine,
        RenderingSystem &renderingSystem,
        Ref<Window> window,
        const DeviceRequirements &deviceRequirements,
        const RenderSettings &renderSettings,
        Result &result
    )
        : m_engine(engine),
          m_renderingSystem(renderingSystem),
          m_window(window),
          m_renderSettings(renderSettings) {
        m_logger = engine.GetLoggerSystem().GetLogger(LoggerSystem::RENDERING_LOGGER);

        if (m_renderSettings.IsValid(m_logger) != Result::SUCCESS) return;

        if (window->CreateSurface(&renderingSystem.GetInstance(), &m_surface) != Result::SUCCESS) {
            result = Result::VULKAN_SURFACE_CREATION_FAILED;
            return;
        }

        m_device = m_engine.GetMemorySystem().New<RenderingDevice>(
            engine,
            renderingSystem,
            *this,
            deviceRequirements,
            result
        );
        m_device->InitializeQueueFamilies();

        if (result != Result::SUCCESS) return;

        m_allocator = m_engine.GetMemorySystem().New<VulkanAllocator>(
            engine,
            *this,
            result
        );
        if (result != Result::SUCCESS) return;

        result = CreateCommandPools();
        if (result != Result::SUCCESS) return;

        m_swapchain = m_engine.GetMemorySystem().New<VulkanSwapchain>(
            engine,
            *this,
            m_device->GetGraphicsQueueFamily(),
            result
        );
        if (result != Result::SUCCESS) return;

        result = Result::SUCCESS;
        m_logger->Verbose("Created renderer");
    }

    Renderer::~Renderer() {
       Result result = DeviceWaitIdle();
        if (result != Result::SUCCESS) {
            m_logger->Warn("Failed to wait for device to be idle before destroying renderer");
        }

        m_swapchain.reset();

        for (const auto &[_, commandPool] : m_commandPools) {
            vkDestroyCommandPool(m_device->GetDevice(), commandPool, nullptr);
        }
        m_commandPools.clear();

        m_allocator.reset();
        m_device.reset();

        if (m_surface) {
            vkDestroySurfaceKHR(m_renderingSystem.GetInstance(), m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
        }

        m_logger->Verbose("Destroyed renderer");
    }

    VkSurfaceKHR Renderer::GetSurface() const {
        return m_surface;
    }

    Ref<RenderingDevice> Renderer::GetDevice() const {
        return m_device;
    }

    Ref<VulkanAllocator> Renderer::GetAllocator() const {
        return m_allocator;
    }

    VulkanSwapchain &Renderer::GetSwapchain() const {
        return *m_swapchain;
    }

    RenderingSystem &Renderer::GetRenderingSystem() const {
        return m_renderingSystem;
    }

    const RenderSettings &Renderer::GetRenderSettings() const {
        return m_renderSettings;
    }

    AllocatedOptional<CommandBuffer> Renderer::CreateCommandBuffer(
        const VulkanQueue &queue, VkCommandBufferLevel level
    ) {
        auto it = m_commandPools.find(queue.GetQueueFamilyIndex());
        PETAL_CHECK_COND(
            it == m_commandPools.end(),
            Result::PETAL_INVALID_QUEUE_FAMILY,
            m_logger,
            "Failed to create command buffer because {} is an invalid queue family", queue.GetQueueFamilyIndex()
        );

        VkCommandPool commandPool = it->second;

        Result result;
        auto commandBuffer = AllocatedOptional<CommandBuffer>::Emplace(m_logger, m_device->GetDevice(), commandPool, level, result);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        return commandBuffer;
    }

    AllocatedOptional<CommandBufferVector> Renderer::CreateCommandBuffers(const VulkanQueue &queue, VkCommandBufferLevel level, glm::u32 count) {
        auto it = m_commandPools.find(queue.GetQueueFamilyIndex());
        PETAL_CHECK_COND(
            it == m_commandPools.end(),
            Result::PETAL_INVALID_QUEUE_FAMILY,
            m_logger,
            "Failed to create command buffer vector because {} is an invalid queue family", queue.GetQueueFamilyIndex()
        );

        VkCommandPool commandPool = it->second;

        Result result;
        auto commandBuffer = AllocatedOptional<CommandBufferVector>::Emplace(m_logger, m_device->GetDevice(), commandPool, level, count, result);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        return commandBuffer;
    }

    Optional<Ref<VulkanFence> > Renderer::CreateFence(VkFenceCreateFlags flags) {
        Result result;
        Ref<VulkanFence> fence = m_engine.GetMemorySystem().New<VulkanFence>(*this, flags, m_logger, result);
        if (result != Result::SUCCESS) return result;
        return fence;
    }

    Optional<std::vector<Ref<VulkanFence> > > Renderer::CreateFences(glm::u32 count, VkFenceCreateFlags flags) {
        PETAL_CHECK_COND(count <= 0, Result::VULKAN_FENCE_CREATION_FAILED, m_logger, "Attempted to create {} fences", count);

        std::vector<Ref<VulkanFence> > fences;
        fences.reserve(count);
        for (glm::u32 i = 0; i < count; i++) {
            Optional<Ref<VulkanFence> > fence = CreateFence(flags);
            PETAL_CHECK_OPTIONAL_SILENT(fence);
            fences.push_back(*fence.Value());
        }

        return fences;
    }

    Optional<Ref<VulkanSemaphore> > Renderer::CreateSemaphore(VkSemaphoreCreateFlags flags) {
        Result result;
        Ref<VulkanSemaphore> semaphore = m_engine.GetMemorySystem().New<VulkanSemaphore>(*this, m_logger, result);
        if (result != Result::SUCCESS) return result;
        return semaphore;
    }

    Optional<std::vector<Ref<VulkanSemaphore> > > Renderer::CreateSemaphores(glm::u32 count, VkSemaphoreCreateFlags flags) {
        PETAL_CHECK_COND(count <= 0, Result::VULKAN_SEMAPHORE_CREATION_FAILED, m_logger, "Attempted to create {} semaphores", count);

        std::vector<Ref<VulkanSemaphore> > semaphores;
        semaphores.reserve(count);

        for (glm::u32 i = 0; i < count; i++) {
            Optional<Ref<VulkanSemaphore> > semaphore = CreateSemaphore(flags);
            PETAL_CHECK_OPTIONAL_SILENT(semaphore);
            semaphores.push_back(*semaphore.Value());
        }

        return semaphores;
    }

    void Renderer::CmdTransitionImage(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        OptionalRef<VkImageMemoryBarrier2> transition
    ) {
        VkImageMemoryBarrier2 imageBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .dstAccessMask = newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ? 0 : VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = GetDevice()->GetGraphicsQueueFamily().GetQueueFamilyIndex(),
            .dstQueueFamilyIndex = GetDevice()->GetGraphicsQueueFamily().GetQueueFamilyIndex(),
            .image = image,
            .subresourceRange = DEFAULT_IMAGE_SUBRESOURCE_RANGE
        };

        VkDependencyInfo depInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = 0,
            .memoryBarrierCount = 0,
            .pMemoryBarriers = nullptr,
            .bufferMemoryBarrierCount = 0,
            .pBufferMemoryBarriers = nullptr,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = transition.HasValue() ? transition.Value() : &imageBarrier
        };

        vkCmdPipelineBarrier2(commandBuffer, &depInfo);
    }

    Result Renderer::DeviceWaitIdle() {
        VkResult result = vkDeviceWaitIdle(m_device->GetDevice());
        PETAL_CHECK_COND(result != VK_SUCCESS, Result::VULKAN_DEVICE_WAIT_IDLE_FAILED, m_logger, "{}", result);
        return Result::SUCCESS;
    }

    Result Renderer::CreateCommandPools() {
        for (VulkanQueue &queue : m_device->GetQueueFamilies()) {
            VkCommandPoolCreateInfo cmdPoolCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queue.GetQueueFamilyIndex()
            };

            VkCommandPool commandPool = VK_NULL_HANDLE;
            VkResult res = vkCreateCommandPool(m_device->GetDevice(), &cmdPoolCreateInfo, nullptr, &commandPool);
            PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_COMMAND_POOL_CREATION_FAILED, m_logger, "Failed to create command pool: {}", res);

            m_commandPools[queue.GetQueueFamilyIndex()] = commandPool;
        }

        return Result::SUCCESS;
    }
} // Petal
