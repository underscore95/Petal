#pragma once
#include "Common.h"
#include "CommandBuffers/CommandBufferRef.h"


namespace Petal {
    struct RenderTarget;
}

namespace Petal {
    class VulkanTexture;
    class VulkanShader;
    class GraphicsContext;
    class VulkanSemaphore;
    class CommandBufferVector;
    class VulkanQueue;
    class VulkanFence;

    class VulkanSwapchain {
    public:
        VulkanSwapchain(
            Engine &engine,
            GraphicsContext &context,
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

        // Submit a command used to render the frame.
        // It will be stored and all frame command buffers will be executed at once at the end of frame
        void SubmitFrameCommand(
            const CommandBufferStrongRef &commandBuffer
        );

        // Submit a command and it will run while blocking
        Result SubmitBlockingCommand(VkCommandBuffer commandBuffer);

        VkSurfaceFormat2KHR GetSurfaceFormat() const;

       const std::vector<RenderTarget> & GetSwapchainRenderTarget() const;

        struct RenderCommandBuffers {
            friend class VulkanSwapchain;
        private:
            RenderCommandBuffers(
                GraphicsContext &context,
                const std::shared_ptr<CommandBufferVector> &commands,
                const std::shared_ptr<std::vector<RenderTarget> > &renderTargets
            );

        public:
            ~RenderCommandBuffers();

            DISABLE_COPY_AND_MOVE(RenderCommandBuffers);

        public:
            const CommandBufferVector& GetCommands() const;

        private:
            GraphicsContext &m_context;
            std::shared_ptr<CommandBufferVector> m_commands;
            std::shared_ptr<std::vector<RenderTarget> > m_renderTargets;
        };

        // This must be called before any render commands are recorded into the command buffer
        // CommandBufferVector should contain NumSwapchainImages() command buffers
        // If renderTargets is empty, render to the swapchain
        std::shared_ptr<RenderCommandBuffers> CmdBeginRendering(
            const std::shared_ptr<CommandBufferVector> &commandBuffers,
            OptionalRef<std::shared_ptr<std::vector<RenderTarget> > > renderTargets = Result::PETAL_OPTIONAL_EMPTY
        ) const;

        // Instanced rendering using a specific shader
        // Vertex/index buffers must be bound to the shader separately
        void CmdRenderIndexed(
            const VulkanShader &shader,
            const RenderCommandBuffers &commandBuffers,
            glm::u32 numIndices,
            glm::u32 numInstances = 1
        ) const;

        // Schedule a function to run after <num swapchain images> frames
        // Note this function will not run if the engine shuts down first however it will run if only the swapchain is destroyed
        void ScheduleSwapchainFrames(const std::function<void()> &function) const;

    private:
        Result CreateSyncObjects();

        Result CreateSwapchain(const VulkanQueue &queue, glm::uvec2 windowSize);

        Result CreateSwapchainImages();

        Optional<glm::u32> CheckRequestedNumImagesSupported();

        Optional<VkPresentModeKHR> CheckRequestedPresentMode() const;

        Result ChooseSurfaceFormat();

        Result CreateDepthBuffer(glm::uvec2 windowSize);

    private:
        Engine &m_engine;
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        VkSwapchainKHR m_handle;
        VkSurfaceFormat2KHR m_swapchainSurfaceFormat;
        glm::u32 m_numSwapchainImages;
        std::vector<VkImageView> m_swapchainImageViews;
        std::shared_ptr<std::vector<RenderTarget> > m_swapchainTargets;
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
