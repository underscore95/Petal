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
#include "Memory/GPUBufferSubsystem.h"
#include "Shaders/IntermediateShaderResource.h"
#include "Shaders/ShaderSubsystem.h"
#include "Internal/VulkanShader.h"

namespace Petal {
    Renderer::Renderer(
        Engine &engine,
        RenderingSystem &renderingSystem,
        std::shared_ptr<Window> window,
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

        m_device = std::make_shared<RenderingDevice>(
            engine,
            renderingSystem,
            *this,
            deviceRequirements,
            result
        );
        m_device->InitializeQueueFamilies();

        if (result != Result::SUCCESS) return;

        m_allocator = std::make_shared<VulkanAllocator>(
            engine,
            *this,
            result
        );
        if (result != Result::SUCCESS) return;

        result = CreateCommandPools();
        if (result != Result::SUCCESS) return;

        result = CreateSwapchain();
        if (result != Result::SUCCESS) return;

        m_bufferSubsystem = std::make_unique<GPUBufferSubsystem>(*this, m_logger, result);
        if (result != Result::SUCCESS) return;

        result = Result::SUCCESS;
        m_logger->Verbose("Created renderer");
    }

    Renderer::~Renderer() {
        m_bufferSubsystem.reset();

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

    Window &Renderer::GetWindow() const {
        return *m_window;
    }

    VkSurfaceKHR Renderer::GetSurface() const {
        return m_surface;
    }

    std::shared_ptr<RenderingDevice> Renderer::GetDevice() const {
        return m_device;
    }

    std::shared_ptr<VulkanAllocator> Renderer::GetAllocator() const {
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

    GPUBufferSubsystem &Renderer::GetBufferSubsystem() const {
        return *m_bufferSubsystem;
    }

    AllocatedOptional<VulkanShader> Renderer::CompileShader(const ShaderAsset &asset) {
        // TODO cache the SPIRV and reflection info

        Optional<IntermediateShaderResource> intermediateShader = m_renderingSystem.GetShaderSubsystem().CompileSlangShader(asset);
        PETAL_CHECK_OPTIONAL_SILENT(intermediateShader);

        Result result;
        auto shader = AllocatedOptional<VulkanShader>::Emplace(
            *this,
            *intermediateShader.Value(),
            m_logger,
            result
        );
        if (result != Result::SUCCESS) return result;
        return shader;
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

    AllocatedOptional<CommandBufferVector> Renderer::CreateCommandBuffers(const VulkanQueue &queueFamily, VkCommandBufferLevel level, glm::u32 count) {
        PETAL_CHECK_COND(
            count == 0,
            Result::VULKAN_COMMAND_BUFFER_CREATION_FAILED,
            m_logger,
            "Attempted to create 0 command buffers"
        );

        auto it = m_commandPools.find(queueFamily.GetQueueFamilyIndex());
        PETAL_CHECK_COND(
            it == m_commandPools.end(),
            Result::PETAL_INVALID_QUEUE_FAMILY,
            m_logger,
            "Failed to create command buffer vector because {} is an invalid queue family", queueFamily.GetQueueFamilyIndex()
        );

        VkCommandPool commandPool = it->second;

        Result result;
        auto commandBuffer = AllocatedOptional<CommandBufferVector>::Emplace(m_logger, m_device->GetDevice(), commandPool, level, count, result);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        return commandBuffer;
    }

    AllocatedOptional<CommandBufferVector> Renderer::CreateCommandBuffersWithContents(
        const VulkanQueue &queueFamily,
        VkCommandBufferLevel level,
        glm::u32 count,
        VkCommandBufferUsageFlags usageFlags,
        const std::function<void(VkCommandBuffer, glm::u32)> &lambda
    ) {
        AllocatedOptional<CommandBufferVector> commandBufferVector = CreateCommandBuffers(queueFamily, level, count);
        if (commandBufferVector.IsEmpty()) return commandBufferVector;

        for (glm::u32 i = 0; i < count; i++) {
            Result result = commandBufferVector.Value()->Begin(i, usageFlags);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            lambda(commandBufferVector.Value()->GetHandle(i), i);

            result = commandBufferVector.Value()->End(i);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
        }

        return commandBufferVector;
    }

    Optional<std::shared_ptr<VulkanFence> > Renderer::CreateFence(VkFenceCreateFlags flags) {
        Result result;
        std::shared_ptr<VulkanFence> fence = std::make_shared<VulkanFence>(*this, flags, m_logger, result);
        if (result != Result::SUCCESS) return result;
        return fence;
    }

    Optional<std::vector<std::shared_ptr<VulkanFence> > > Renderer::CreateFences(glm::u32 count, VkFenceCreateFlags flags) {
        PETAL_CHECK_COND(count <= 0, Result::VULKAN_FENCE_CREATION_FAILED, m_logger, "Attempted to create {} fences", count);

        std::vector<std::shared_ptr<VulkanFence> > fences;
        fences.reserve(count);
        for (glm::u32 i = 0; i < count; i++) {
            Optional<std::shared_ptr<VulkanFence> > fence = CreateFence(flags);
            PETAL_CHECK_OPTIONAL_SILENT(fence);
            fences.push_back(*fence.Value());
        }

        return fences;
    }

    Optional<std::shared_ptr<VulkanSemaphore> > Renderer::CreateSemaphore(VkSemaphoreCreateFlags flags) {
        Result result;
        std::shared_ptr<VulkanSemaphore> semaphore = std::make_shared<VulkanSemaphore>(*this, m_logger, result);
        if (result != Result::SUCCESS) return result;
        return semaphore;
    }

    Optional<std::vector<std::shared_ptr<VulkanSemaphore> > > Renderer::CreateSemaphores(glm::u32 count, VkSemaphoreCreateFlags flags) {
        PETAL_CHECK_COND(count <= 0, Result::VULKAN_SEMAPHORE_CREATION_FAILED, m_logger, "Attempted to create {} semaphores", count);

        std::vector<std::shared_ptr<VulkanSemaphore> > semaphores;
        semaphores.reserve(count);

        for (glm::u32 i = 0; i < count; i++) {
            Optional<std::shared_ptr<VulkanSemaphore> > semaphore = CreateSemaphore(flags);
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

    Result Renderer::RecreateSwapchain() {
        Result result = DeviceWaitIdle();
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        m_swapchain.reset();

        result = CreateSwapchain();
        return result;
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

    Result Renderer::CreateSwapchain() {
        Result result;
        m_swapchain = std::make_shared<VulkanSwapchain>(
            m_engine,
            *this,
            m_device->GetGraphicsQueueFamily(),
            result
        );
        return result;
    }
} // Petal
