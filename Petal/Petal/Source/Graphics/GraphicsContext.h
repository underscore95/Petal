#pragma once

#include <vulkan/vulkan.h>
#include "Common.h"
#include "GraphicsSettings.h"
#include "Internal/DeviceRequirements.h"
#include "Internal/VulkanGraphicsPipeline.h"
#include "Other/IndexType.h"
#include "Rendering/RendererSettings.h"

namespace Petal {
    class GPUBuffer;
}

namespace Petal {
    class Renderer;
    class GPUMemorySubsystem;
    class VulkanShader;
    struct ShaderAsset;
    class VulkanQueue;
    class VulkanSemaphore;
    class VulkanFence;
    class VulkanSwapchain;
    class CommandBuffer;
    class CommandBufferVector;
    class Engine;
    class Window;
    class GraphicsSystem;
    class VulkanAllocator;
    class RenderingDevice;

    class GraphicsContext {
    public:
        // Create via GraphicsSystem
        GraphicsContext(
            Engine &engine,
            GraphicsSystem &renderingSystem,
            std::shared_ptr<Window> window,
            const DeviceRequirements &deviceRequirements,
            const GraphicsSettings &graphicsSettings,
            Result &result
        );

        ~GraphicsContext();

        DISABLE_COPY_AND_MOVE(GraphicsContext);

    public:
        Engine &GetEngine() const;

        Window &GetWindow() const;

        VkSurfaceKHR GetSurface() const;

        std::shared_ptr<RenderingDevice> GetDevice() const;

        std::shared_ptr<VulkanAllocator> GetAllocator() const;

        VulkanSwapchain &GetSwapchain() const;

        GraphicsSystem &GetRenderingSystem() const;

        const GraphicsSettings &GetGraphicsSettings() const;

        GPUMemorySubsystem &GetMemorySubsystem() const;

        AllocatedOptional<VulkanShader> CompileShader(const ShaderAsset &asset);

        // Set the debug name of a vulkan object
        // In release mode, this is a no op
        void SetObjectDebugName(glm::u64 handle, VkObjectType objectType, const std::string &objectName) const {
#ifndef NDEBUG
            SetObjectDebugNameImpl(handle, objectType, objectName);
#endif
        }

        // Create a command buffer.
        // It is recommended to move the command buffer into a shared ptr after creation so it can be converted into a CommandBufferRef
        AllocatedOptional<CommandBuffer> CreateCommandBuffer(
            const VulkanQueue &queueFamily,
            VkCommandBufferLevel level
        );

        // Create N command buffers.
        // It is recommended to move the command buffers into a shared ptr after creation so it can be converted into a CommandBufferRef
        AllocatedOptional<CommandBufferVector> CreateCommandBuffers(
            const VulkanQueue &queueFamily,
            VkCommandBufferLevel level,
            glm::u32 count
        );

        // Create N command buffers.
        // It is recommended to move the command buffers into a shared ptr after creation so it can be converted into a CommandBufferRef
        // The lambda is executed once per command buffer to record, you do not need to begin/end inside this lambda
        AllocatedOptional<CommandBufferVector> CreateCommandBuffersWithContents(
            const VulkanQueue &queueFamily,
            VkCommandBufferLevel level,
            glm::u32 count,
            VkCommandBufferUsageFlags usageFlags,
            const std::function<void(VkCommandBuffer, glm::u32)> &lambda
        );

        Optional<std::shared_ptr<VulkanFence> > CreateFence(VkFenceCreateFlags flags = 0);

        Optional<std::vector<std::shared_ptr<VulkanFence> > > CreateFences(glm::u32 count, VkFenceCreateFlags flags = 0);

        Optional<std::shared_ptr<VulkanSemaphore> > CreateSemaphore(VkSemaphoreCreateFlags flags = 0);

        Optional<std::vector<std::shared_ptr<VulkanSemaphore> > > CreateSemaphores(glm::u32 count, VkSemaphoreCreateFlags flags = 0);

        AllocatedOptional<Renderer> CreateRenderer(const RendererSettings &settings);

        // Shorthand for transitioning an image layout
        // By default, an excessively blocking barrier for graphics queue images is used
        void CmdTransitionImage(
            VkCommandBuffer commandBuffer,
            VkImage image,
            VkImageLayout oldLayout,
            VkImageLayout newLayout
        );

        void CmdTransitionImage(
            VkCommandBuffer commandBuffer,
            VkImageMemoryBarrier2 transition
        );

        Result DeviceWaitIdle();

        // Wait until the device is idle and then recreate the swapchain.
        // This is required if the window is resized.
        Result RecreateSwapchain();

        void CmdWritePushConstants(
            const CommandBufferVector &commandBuffers,
            const VulkanGraphicsPipeline &pipeline,
            const void *data, glm::u32 size
        ) const;

        // Bind one or more vertex buffers
        void CmdBindVertexBuffer(
            const CommandBufferVector &commandBuffers,
            glm::u32 firstBinding,
            const std::vector<std::reference_wrapper<const GPUBuffer> > &buffers
        ) const;

        // Bind an index buffer
        void CmdBindIndexBuffer(
            const CommandBufferVector &commandBuffers,
            const GPUBuffer &buffer,
            IndexType indexType
        ) const;

        static constexpr VkImageSubresourceRange DEFAULT_IMAGE_COLOR_SUBRESOURCE_RANGE = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        static constexpr VkImageSubresourceRange DEFAULT_IMAGE_DEPTH_SUBRESOURCE_RANGE = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

    private:
        Result CreateCommandPools();

        Result CreateSwapchain();

#ifndef NDEBUG
        void SetObjectDebugNameImpl(glm::u64 handle, VkObjectType objectType, const std::string &objectName) const;
#endif

    private:
        Engine &m_engine;
        GraphicsSystem &m_graphicsSystem;
        std::shared_ptr<Window> m_window;

        VkSurfaceKHR m_surface;
        std::shared_ptr<RenderingDevice> m_device;
        std::shared_ptr<Logger> m_logger;
        std::shared_ptr<VulkanAllocator> m_allocator;
        GraphicsSettings m_graphicsSettings;
        // Queue family -> command pool
        std::unordered_map<glm::u32, VkCommandPool> m_commandPools;
        std::shared_ptr<VulkanSwapchain> m_swapchain;
        std::unique_ptr<GPUMemorySubsystem> m_memorySubsystem;
    };
} // Petal
