#include "GraphicsContext.h"
#include "GraphicsSystem.h"
#include "Engine.h"
#include "Window/Window.h"
#include "Memory/MemorySystem.h"
#include "Internal/VulkanAllocator.h"
#include "Internal/VulkanQueue.h"
#include "Internal/CommandBuffers/CommandBuffer.h"
#include "Internal/CommandBuffers/CommandBufferVector.h"
#include "Internal/RenderingDevice.h"
#include "Internal/VulkanGraphicsPipeline.h"
#include "Internal/VulkanSwapchain.h"
#include "Internal/Sync/VulkanFence.h"
#include "Internal/Sync/VulkanSemaphore.h"
#include "Memory/GPUMemorySubsystem.h"
#include "Shaders/IntermediateShaderResource.h"
#include "Shaders/ShaderSubsystem.h"
#include "Internal/VulkanShader.h"
#include "Rendering/Renderer.h"

namespace Petal {
    GraphicsContext::GraphicsContext(
        Engine &engine,
        GraphicsSystem &renderingSystem,
        std::shared_ptr<Window> window,
        const DeviceRequirements &deviceRequirements,
        const GraphicsSettings &graphicsSettings,
        Result &result
    )
        : m_engine(engine),
          m_graphicsSystem(renderingSystem),
          m_window(window),
          m_graphicsSettings(graphicsSettings) {
        m_logger = engine.GetLoggerSystem().GetLogger(LoggerSystem::GRAPHICS_LOGGER);

        if (m_graphicsSettings.IsValid(m_logger) != Result::SUCCESS) return;

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

        m_memorySubsystem = std::make_unique<GPUMemorySubsystem>(*this, m_logger, result);
        if (result != Result::SUCCESS) return;

        result = Result::SUCCESS;
        m_logger->Verbose("Created renderer");
    }

    GraphicsContext::~GraphicsContext() {
        m_memorySubsystem.reset();

        Result result = DeviceWaitIdle();
        if (result != Result::SUCCESS) {
            m_logger->Warn("Failed to wait for device to be idle before destroying renderer");
        }

        m_swapchain.reset();

        for (const auto &[_, commandPool] : m_commandPools) {
            vkDestroyCommandPool(m_device->GetHandle(), commandPool, nullptr);
        }
        m_commandPools.clear();

        m_allocator.reset();
        m_device.reset();

        if (m_surface) {
            vkDestroySurfaceKHR(m_graphicsSystem.GetInstance(), m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
        }

        m_logger->Verbose("Destroyed renderer");
    }

    Window &GraphicsContext::GetWindow() const {
        return *m_window;
    }

    VkSurfaceKHR GraphicsContext::GetSurface() const {
        return m_surface;
    }

    std::shared_ptr<RenderingDevice> GraphicsContext::GetDevice() const {
        return m_device;
    }

    std::shared_ptr<VulkanAllocator> GraphicsContext::GetAllocator() const {
        return m_allocator;
    }

    VulkanSwapchain &GraphicsContext::GetSwapchain() const {
        return *m_swapchain;
    }

    GraphicsSystem &GraphicsContext::GetRenderingSystem() const {
        return m_graphicsSystem;
    }

    const GraphicsSettings &GraphicsContext::GetGraphicsSettings() const {
        return m_graphicsSettings;
    }

    GPUMemorySubsystem &GraphicsContext::GetMemorySubsystem() const {
        return *m_memorySubsystem;
    }

    AllocatedOptional<VulkanShader> GraphicsContext::CompileShader(const ShaderAsset &asset) {
        // TODO cache the SPIRV and reflection info

        Optional<IntermediateShaderResource> intermediateShader = m_graphicsSystem.GetShaderSubsystem().CompileSlangShader(asset);
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

#ifndef NDEBUG
    void GraphicsContext::SetObjectDebugNameImpl(glm::u64 handle, VkObjectType objectType, const std::string &objectName) const {
        VkDebugUtilsObjectNameInfoEXT info = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = objectType,
            .objectHandle = handle,
            .pObjectName = objectName.c_str()
        };
        m_graphicsSystem.VulkanSetDebugObjectNameFunction()(m_device->GetHandle(), &info);
    }
#endif

    AllocatedOptional<CommandBuffer> GraphicsContext::CreateCommandBuffer(
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
        auto commandBuffer = AllocatedOptional<CommandBuffer>::Emplace(m_logger, m_device->GetHandle(), commandPool, level, result);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        return commandBuffer;
    }

    AllocatedOptional<CommandBufferVector> GraphicsContext::CreateCommandBuffers(const VulkanQueue &queueFamily, VkCommandBufferLevel level, glm::u32 count) {
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
        auto commandBuffer = AllocatedOptional<CommandBufferVector>::Emplace(m_logger, *this, commandPool, level, count, result);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        return commandBuffer;
    }

    AllocatedOptional<CommandBufferVector> GraphicsContext::CreateCommandBuffersWithContents(
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

    Optional<std::shared_ptr<VulkanFence> > GraphicsContext::CreateFence(VkFenceCreateFlags flags) {
        Result result;
        std::shared_ptr<VulkanFence> fence = std::make_shared<VulkanFence>(*this, flags, m_logger, result);
        if (result != Result::SUCCESS) return result;
        return fence;
    }

    Optional<std::vector<std::shared_ptr<VulkanFence> > > GraphicsContext::CreateFences(glm::u32 count, VkFenceCreateFlags flags) {
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

    Optional<std::shared_ptr<VulkanSemaphore> > GraphicsContext::CreateSemaphore(VkSemaphoreCreateFlags flags) {
        Result result;
        std::shared_ptr<VulkanSemaphore> semaphore = std::make_shared<VulkanSemaphore>(*this, m_logger, result);
        if (result != Result::SUCCESS) return result;
        return semaphore;
    }

    Optional<std::vector<std::shared_ptr<VulkanSemaphore> > > GraphicsContext::CreateSemaphores(glm::u32 count, VkSemaphoreCreateFlags flags) {
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

    AllocatedOptional<Renderer> GraphicsContext::CreateRenderer(
        const RendererSettings &settings
    ) {
        Result resultOut;
        auto renderer = std::make_unique<Renderer>(*this, m_logger, settings, resultOut);
        PETAL_CHECK_COND_SILENT(resultOut != Result::SUCCESS, resultOut);
        return renderer;
    }

    void GraphicsContext::CmdTransitionImage(
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
            .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = GetDevice()->GetGraphicsQueueFamily().GetQueueFamilyIndex(),
            .dstQueueFamilyIndex = GetDevice()->GetGraphicsQueueFamily().GetQueueFamilyIndex(),
            .image = image,
            .subresourceRange = DEFAULT_IMAGE_SUBRESOURCE_RANGE
        };
        if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) imageBarrier.dstAccessMask = 0;

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

    Result GraphicsContext::DeviceWaitIdle() {
        VkResult result = vkDeviceWaitIdle(m_device->GetHandle());
        PETAL_CHECK_COND(result != VK_SUCCESS, Result::VULKAN_DEVICE_WAIT_IDLE_FAILED, m_logger, "{}", result);
        return Result::SUCCESS;
    }

    Result GraphicsContext::RecreateSwapchain() {
        Result result = DeviceWaitIdle();
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        m_swapchain.reset();

        m_device->QueryDeviceSurfaceCapabilities();
        result = CreateSwapchain();
        return result;
    }

    void GraphicsContext::CmdWritePushConstants(
        const CommandBufferVector &commandBuffers,
        const VulkanShader &shader,
        const void *data,
        glm::u32 size
    ) {
        assert(commandBuffers.IsSwapchainSize());
        for (glm::u32 i = 0; i < commandBuffers.Size(); i++) {
            vkCmdPushConstants(
                commandBuffers.GetHandle(i),
                shader.GetPipeline().GetLayout(),
                VK_SHADER_STAGE_ALL,
                0,
                size,
                data
            );
        }
    }

    void GraphicsContext::CmdBindVertexBuffer(
        const CommandBufferVector &commandBuffers,
        glm::u32 firstBinding,
        const std::vector<std::reference_wrapper<const GPUBuffer> > &buffers
    ) const {
        assert(commandBuffers.IsSwapchainSize());
        if (buffers.empty()) {
            m_logger->Warn("Binding 0 vertex buffers");
        }

        std::vector<VkBuffer> bufferHandles;
        bufferHandles.reserve(buffers.size());
        std::vector<VkDeviceSize> bufferOffsets;
        bufferOffsets.reserve(buffers.size());
        for (std::reference_wrapper<const GPUBuffer> buffer : buffers) {
            bufferHandles.push_back(buffer.get().GetBackingBuffer()->GetHandle());
            bufferOffsets.push_back(buffer.get().GetAllocation().Location);
        }

        for (glm::u32 i = 0; i < commandBuffers.Size(); i++) {
            vkCmdBindVertexBuffers(
                commandBuffers.GetHandle(i),
                firstBinding,
                buffers.size(),
                bufferHandles.data(),
                bufferOffsets.data()
            );
        }
    }

    void GraphicsContext::CmdBindIndexBuffer(
        const CommandBufferVector &commandBuffers,
        const GPUBuffer &buffer,
        IndexType indexType
    ) const {
        assert(indexType == IndexType::INDICES_32_BIT); // for now only 32 bit supported

        for (glm::u32 i = 0; i < commandBuffers.Size(); i++) {
            vkCmdBindIndexBuffer(
                commandBuffers.GetHandle(i),
                buffer.GetBackingBuffer()->GetHandle(),
                buffer.GetAllocation().Location,
                IndexTypes::GetData(indexType).VulkanIndexType
            );
        }
    }

    Result GraphicsContext::CreateCommandPools() {
        for (VulkanQueue &queue : m_device->GetQueueFamilies()) {
            VkCommandPoolCreateInfo cmdPoolCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queue.GetQueueFamilyIndex()
            };

            VkCommandPool commandPool = VK_NULL_HANDLE;
            VkResult res = vkCreateCommandPool(m_device->GetHandle(), &cmdPoolCreateInfo, nullptr, &commandPool);
            PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_COMMAND_POOL_CREATION_FAILED, m_logger, "Failed to create command pool: {}", res);

            m_commandPools[queue.GetQueueFamilyIndex()] = commandPool;
        }

        return Result::SUCCESS;
    }

    Result GraphicsContext::CreateSwapchain() {
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
