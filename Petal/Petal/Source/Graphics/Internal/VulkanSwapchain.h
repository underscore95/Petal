#pragma once
#include "Common.h"
#include "CommandBuffers/CommandBufferRef.h"


namespace Petal {
    class VulkanShader;
}

namespace Petal {
    class GraphicsContext;
    class VulkanSemaphore;
    class CommandBufferVector;
    class VulkanQueue;
    class VulkanFence;

    class VulkanSwapchain {
    public:
        VulkanSwapchain(
            Engine &engine,
            GraphicsContext &renderer,
            const VulkanQueue &queueFamily,
            Result &resultOut
        );

        ~VulkanSwapchain();

    public:
        VkSwapchainKHR GetHandle() const;

        glm::u32 NumSwapchainImages() const;

        // Must be called before rendering begins each frame.
        // This function will return PETAL_WINDOW_RESIZED if the window has been resized and the swapchain must be recreated.
        // You must also call CmdBeginRendering on any command buffer used for rendering
        Result BeginRendering();

        // Must be called after rendering completes each frame.
        // This function will return PETAL_WINDOW_RESIZED if the window has been resized and the swapchain must be recreated.
        Result EndRendering();

        // Index of the image in the swapchain we are rendering to for the current frame.
        // This is guaranteed to be between 0 and NumSwapchainImages()-1
        glm::u32 GetSwapchainIndex() const;

        // Record a command to clear a specific swapchain image into a command buffer
        void CmdClear(
            VkCommandBuffer commandBuffer,
            const Color &color,
            glm::u32 swapchainIndex
        ) const;

        // Submit a command used to render the frame.
        // It will be stored and all frame command buffers will be executed at once at the end of frame
        void SubmitFrameCommand(
            const CommandBufferStrongRef &commandBuffer
        );

        // Submit a command and it will run while blocking
        Result SubmitBlockingCommand(VkCommandBuffer commandBuffer);

        VkSurfaceFormat2KHR GetSurfaceFormat() const;

        // This must be called before any render commands are recorded into the command buffer
        // CommandBufferVector should contain NumSwapchainImages() command buffers
        void CmdBeginRendering(const CommandBufferVector &commandBuffers) const;

        // This must be called once all render commands have been recorded into the command buffer, if CmdBeginRendering has been called.
        // CommandBufferVector should contain NumSwapchainImages() command buffers
        void CmdEndRendering(const CommandBufferVector &commandBuffers) const;

        // Instanced rendering using a specific shader
        // Bind the vertex buffer and index buffer (if using) to the shader before submitting the command buffer.
        void CmdRenderIndexed(
            const VulkanShader &shader,
            const CommandBufferVector &commandBuffers,
            glm::u32 numIndices,
            glm::u32 numInstances = 1
        );

        // Schedule a function to run after <num swapchain images> frames
        // Note this function will not run if the engine shuts down first however it will run if only the swapchain is destroyed
        void ScheduleSwapchainFrames(const std::function<void()>& function) const;

    private:
        Result CreateSyncObjects();

        Result CreateSwapchain(const VulkanQueue &queue, glm::uvec2 windowSize);

        Result CreateSwapchainImages();

        Optional<glm::u32> CheckRequestedNumImagesSupported();

        Optional<VkPresentModeKHR> CheckRequestedPresentMode() const;

        Result ChooseSurfaceFormat();

    private:
        Engine& m_engine;
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        VkSwapchainKHR m_handle;
        VkSurfaceFormat2KHR m_swapchainSurfaceFormat;
        glm::u32 m_numSwapchainImages;
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;
        std::shared_ptr<VulkanFence> m_blockingCommandFence;

        // Frame data
        glm::u32 m_swapchainIndex = 0;
        std::vector<std::shared_ptr<VulkanFence> > m_frameCompleteFences;
        std::vector<std::shared_ptr<VulkanSemaphore> > m_frameCompleteSemaphores;
        std::vector<std::shared_ptr<VulkanSemaphore> > m_swapchainSemaphores;
        std::vector<CommandBufferStrongRef> m_submittedFrameCommands;
        std::vector<VkCommandBufferSubmitInfo> m_submittedFrameCommandSubmitInfos;
    };
} // Petal
