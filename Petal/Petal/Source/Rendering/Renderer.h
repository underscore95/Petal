#pragma once

#include <vulkan/vulkan.h>
#include "Common.h"
#include "RenderSettings.h"
#include "Internal/DeviceRequirements.h"

namespace Petal {
    class VulkanQueue;
    class VulkanSemaphore;
    class VulkanFence;
    class VulkanSwapchain;
    class CommandBuffer;
    class CommandBufferVector;
    class Engine;
    class Window;
    class RenderingSystem;
    class VulkanAllocator;
    class RenderingDevice;

    class Renderer {
    public:
        // Create via RenderingSystem
        Renderer(
            Engine &engine,
            RenderingSystem &renderingSystem,
            std::shared_ptr<Window> window,
            const DeviceRequirements &deviceRequirements,
            const RenderSettings &renderSettings,
            Result &result
        );

        ~Renderer();

        DISABLE_COPY_AND_MOVE(Renderer);

    public:
        Window &GetWindow() const;

        VkSurfaceKHR GetSurface() const;

        std::shared_ptr<RenderingDevice> GetDevice() const;

        std::shared_ptr<VulkanAllocator> GetAllocator() const;

        VulkanSwapchain &GetSwapchain() const;

        RenderingSystem &GetRenderingSystem() const;

        const RenderSettings &GetRenderSettings() const;

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

        Optional<std::shared_ptr<VulkanFence> > CreateFence(VkFenceCreateFlags flags = 0);

        Optional<std::vector<std::shared_ptr<VulkanFence> > > CreateFences(glm::u32 count, VkFenceCreateFlags flags = 0);

        Optional<std::shared_ptr<VulkanSemaphore> > CreateSemaphore(VkSemaphoreCreateFlags flags = 0);

        Optional<std::vector<std::shared_ptr<VulkanSemaphore> > > CreateSemaphores(glm::u32 count, VkSemaphoreCreateFlags flags = 0);

        // Shorthand for transitioning an image layout
        // By default, an excessively blocking barrier for graphics queue images is used, but this can be overridden by passing a transition parameter
        void CmdTransitionImage(
            VkCommandBuffer commandBuffer,
            VkImage image,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            OptionalRef<VkImageMemoryBarrier2> transition = OptionalRef<VkImageMemoryBarrier2>::Empty()
        );

        Result DeviceWaitIdle();

        // Wait until the device is idle and then recreate the swapchain.
        // This is required if the window is resized.
        Result RecreateSwapchain();

        static constexpr VkImageSubresourceRange DEFAULT_IMAGE_SUBRESOURCE_RANGE = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

    private:
        Result CreateCommandPools();

        Result CreateSwapchain();

    private:
        Engine &m_engine;
        RenderingSystem &m_renderingSystem;
        std::shared_ptr<Window> m_window;

        VkSurfaceKHR m_surface;
        std::shared_ptr<RenderingDevice> m_device;
        std::shared_ptr<Logger> m_logger;
        std::shared_ptr<VulkanAllocator> m_allocator;
        RenderSettings m_renderSettings;
        // Queue family -> command pool
        std::unordered_map<glm::u32, VkCommandPool> m_commandPools;
        std::shared_ptr<VulkanSwapchain> m_swapchain;
    };
} // Petal
