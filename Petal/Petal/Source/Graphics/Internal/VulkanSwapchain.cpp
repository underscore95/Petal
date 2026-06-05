#include "VulkanSwapchain.h"
#include "Graphics/GraphicsContext.h"
#include "Engine.h"
#include "RenderingDevice.h"
#include "RenderTarget.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanQueue.h"
#include "VulkanShader.h"
#include "CommandBuffers/CommandBufferVector.h"
#include "Graphics/Memory/GPUMemorySubsystem.h"
#include "Sync/VulkanFence.h"
#include "Sync/VulkanSemaphore.h"
#include "Window/Window.h"

namespace Petal {
    struct SwapchainImage final : ITexture {
        VkImage Image;
        VkImageView View;
        std::string Name;

        VkImageView GetImageView() const override {
            return View;
        }

        VkImage GetImage() const override { return Image; }

        const std::string &GetName() const override { return Name; }

        VkImageSubresourceRange GetRange() const override {
            return {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };
        }
    };

    VulkanSwapchain::VulkanSwapchain(
        Engine &engine,
        GraphicsContext &context,
        const VulkanQueue &queueFamily,
        Result &resultOut
    ) : m_engine(engine),
        m_context(context) {
        m_logger = engine.GetLoggerSystem().GetLogger(LoggerSystem::GRAPHICS_LOGGER);

        glm::uvec2 windowSize = context.GetWindow().GetDimensions();
        resultOut = CreateSwapchain(queueFamily, windowSize);
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateSwapchainImages();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateSyncObjects();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateDepthBuffer(windowSize);
        if (resultOut != Result::SUCCESS) return;

        m_logger->Verbose("Created swapchain for window size {}x{}", windowSize.x, windowSize.y);
    }

    VulkanSwapchain::~VulkanSwapchain() {
        for (const RenderTarget &renderTarget : *m_swapchainTargets) {
            // color image is part of VkSwapchain
            // depth buffer is destroyed by VulkanTexture destructor
            vkDestroyImageView(m_context.GetDevice()->GetHandle(), renderTarget.Color->GetImageView(), nullptr);
        }

        vkDestroySwapchainKHR(
            m_context.GetDevice()->GetHandle(),
            m_handle,
            nullptr
        );

        m_logger->Verbose("Destroyed swapchain");
    }

    VkSwapchainKHR VulkanSwapchain::GetHandle() const {
        return m_handle;
    }

    glm::u32 VulkanSwapchain::NumSwapchainImages() const {
        return m_numSwapchainImages;
    }

    Result VulkanSwapchain::BeginRendering() {
        // Wait for previous frame to complete
        VkFence frameCompleteFence = m_frameCompleteFences[m_swapchainIndex]->GetHandle();
        VkResult res = vkWaitForFences(m_context.GetDevice()->GetHandle(), 1, &frameCompleteFence, true, PETAL_U64_MAX);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_BEGIN_RENDER_FAILED, m_logger, "Failed to wait for fence: {}", res);

        res = vkResetFences(m_context.GetDevice()->GetHandle(), 1, &frameCompleteFence);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_BEGIN_RENDER_FAILED, m_logger, "Failed to reset fence: {}", res);

        // Request new frame
        res = vkAcquireNextImageKHR(
            m_context.GetDevice()->GetHandle(),
            m_handle,
            PETAL_U64_MAX,
            m_swapchainSemaphores[m_swapchainIndex]->GetHandle(),
            VK_NULL_HANDLE,
            &m_swapchainIndex
        );
        PETAL_CHECK_COND_SILENT(res == VK_ERROR_OUT_OF_DATE_KHR, Result::PETAL_WINDOW_RESIZED);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_BEGIN_RENDER_FAILED, m_logger, "Failed acquire image with current swapchain index {}: {}", m_swapchainIndex, res);

        return Result::SUCCESS;
    }

    Result VulkanSwapchain::EndRendering() {
        VkSemaphoreSubmitInfo waitInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = m_swapchainSemaphores[GetSwapchainIndex()]->GetHandle(),
            .value = 1,
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR | VK_PIPELINE_STAGE_2_CLEAR_BIT,
            .deviceIndex = 0
        };

        VkSemaphoreSubmitInfo signalInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = m_frameCompleteSemaphores[GetSwapchainIndex()]->GetHandle(),
            .value = 1,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
            .deviceIndex = 0
        };

        VkSubmitInfo2 submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .flags = 0,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &waitInfo,
            .commandBufferInfoCount = static_cast<glm::u32>(m_submittedFrameCommandSubmitInfos.size()),
            .pCommandBufferInfos = m_submittedFrameCommandSubmitInfos.data(),
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &signalInfo
        };

        VkResult result = vkQueueSubmit2(
            m_context.GetDevice()->GetGraphicsQueueFamily().GetHandle(),
            1,
            &submit,
            m_frameCompleteFences[GetSwapchainIndex()]->GetHandle()
        );

        PETAL_CHECK_COND(result != VK_SUCCESS, Result::PETAL_FRAME_COMMAND_SUBMIT_FAILED, m_logger, "{}", result);

        VkSemaphore frameCompleteSemaphore = m_frameCompleteSemaphores[m_swapchainIndex]->GetHandle();

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frameCompleteSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &m_handle,
            .pImageIndices = &m_swapchainIndex,
            .pResults = nullptr
        };

        VkResult res = vkQueuePresentKHR(m_context.GetDevice()->GetGraphicsQueueFamily().GetHandle(), &presentInfo);
        PETAL_CHECK_COND_SILENT(res == VK_ERROR_OUT_OF_DATE_KHR, Result::PETAL_WINDOW_RESIZED);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_END_RENDER_FAILED, m_logger, "Failed to present: {}", res);

        m_swapchainIndex = (m_swapchainIndex + 1) % m_numSwapchainImages;

        m_submittedFrameCommandSubmitInfos.clear();
        m_submittedFrameCommands.clear();

        return Result::SUCCESS;
    }

    glm::u32 VulkanSwapchain::GetSwapchainIndex() const {
        return m_swapchainIndex;
    }

    void VulkanSwapchain::SubmitFrameCommand(
        const CommandBufferStrongRef &commandBuffer
    ) {
        m_submittedFrameCommands.push_back(commandBuffer);

        VkCommandBufferSubmitInfo commandInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBuffer = commandBuffer.GetHandle(),
            .deviceMask = 0
        };
        m_submittedFrameCommandSubmitInfos.push_back(commandInfo);
    }

    Result VulkanSwapchain::SubmitBlockingCommand(VkCommandBuffer commandBuffer) {
        VkCommandBufferSubmitInfo commandInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBuffer = commandBuffer,
            .deviceMask = 0
        };

        VkSubmitInfo2 submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .flags = 0,
            .waitSemaphoreInfoCount = 0,
            .pWaitSemaphoreInfos = nullptr,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &commandInfo,
            .signalSemaphoreInfoCount = 0,
            .pSignalSemaphoreInfos = nullptr
        };

        VkFence fence = m_blockingCommandFence->GetHandle();
        VkResult result = vkQueueSubmit2(
            m_context.GetDevice()->GetGraphicsQueueFamily().GetHandle(),
            1,
            &submit,
            fence
        );
        PETAL_CHECK_COND(
            result != VK_SUCCESS,
            Result::PETAL_COMMAND_SUBMIT_FAILED,
            m_logger,
            "Failed to submit blocking command: {}", result
        );

        result = vkWaitForFences(
            m_context.GetDevice()->GetHandle(),
            1,
            &fence,
            true,
            1000000000
        );
        PETAL_CHECK_COND(
            result != VK_SUCCESS,
            Result::PETAL_COMMAND_SUBMIT_FAILED,
            m_logger,
            "Failed to wait for blocking command: {}", result
        );

        result = vkResetFences(
            m_context.GetDevice()->GetHandle(),
            1,
            &fence
        );
        PETAL_CHECK_COND(
            result != VK_SUCCESS,
            Result::PETAL_COMMAND_SUBMIT_FAILED,
            m_logger,
            "Failed to reset fence for blocking command: {}", result
        );

        return Result::SUCCESS;
    }

    VkSurfaceFormat2KHR VulkanSwapchain::GetSurfaceFormat() const {
        return m_swapchainSurfaceFormat;
    }

    const std::vector<RenderTarget> &VulkanSwapchain::GetSwapchainRenderTarget() const {
        return *m_swapchainTargets;
    }

    VulkanSwapchain::RenderCommandBuffers::RenderCommandBuffers(
        GraphicsContext &context,
        const std::shared_ptr<CommandBufferVector> &commands,
        const std::shared_ptr<std::vector<RenderTarget> > &renderTargets
    )
        : m_context(context),
          m_commands(commands),
          m_renderTargets(renderTargets) {
        assert(commands->IsSwapchainSize());

        for (glm::u32 swapchainIndex = 0; swapchainIndex < commands->Size(); swapchainIndex++) {
            const RenderTarget &renderTarget = (*m_renderTargets)[swapchainIndex];
            VkCommandBuffer commandBuffer = commands->GetHandle(swapchainIndex);

            glm::uvec2 windowSize = m_context.GetWindow().GetDimensions();

            Optional<VkRenderingAttachmentInfo> colorAttachment = renderTarget.CreateColorAttachment();
            Optional<VkRenderingAttachmentInfo> depthAttachment = renderTarget.CreateDepthAttachment();

            VkRenderingInfo renderingInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderArea = {{0, 0}, {windowSize.x, windowSize.y}},
                .layerCount = 1,
                .viewMask = 0,
                .colorAttachmentCount = 1,
                .pColorAttachments = colorAttachment.HasValue() ? colorAttachment.Data() : nullptr,
                .pDepthAttachment = depthAttachment.HasValue() ? depthAttachment.Data() : nullptr,
                .pStencilAttachment = nullptr
            };
            vkCmdBeginRendering(commandBuffer, &renderingInfo);

            VkViewport viewport = {
                .x = 0,
                .y = 0,
                .width = static_cast<float>(windowSize.x),
                .height = static_cast<float>(windowSize.y),
                .minDepth = 0,
                .maxDepth = 1
            };

            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &renderingInfo.renderArea);
        }
    }

    VulkanSwapchain::RenderCommandBuffers::~RenderCommandBuffers() {
        assert(m_commands->IsSwapchainSize());

        for (glm::u32 swapchainIndex = 0; swapchainIndex < m_commands->Size(); swapchainIndex++) {
            VkCommandBuffer commandBuffer = m_commands->GetHandle(swapchainIndex);

            vkCmdEndRendering(commandBuffer);
        }
    }

    const CommandBufferVector &VulkanSwapchain::RenderCommandBuffers::GetCommands() const {
        return *m_commands;
    }

    std::shared_ptr<VulkanSwapchain::RenderCommandBuffers> VulkanSwapchain::CmdBeginRendering(
        const std::shared_ptr<CommandBufferVector> &commandBuffers,
        OptionalRef<std::shared_ptr<std::vector<RenderTarget> > > renderTargets
    ) const {
        return std::shared_ptr<RenderCommandBuffers>(new RenderCommandBuffers(
            m_context,
            commandBuffers,
            renderTargets.HasValue() ? *renderTargets : m_swapchainTargets
        ));
    }

    void VulkanSwapchain::CmdRenderIndexed(
        const VulkanShader &shader,
        const RenderCommandBuffers &commandBuffers,
        glm::u32 numIndices,
        glm::u32 numInstances
    ) const {
        for (glm::u32 swapchainIndex = 0; swapchainIndex < commandBuffers.GetCommands().Size(); swapchainIndex++) {
            VkCommandBuffer commandBuffer = commandBuffers.GetCommands().GetHandle(swapchainIndex);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.GetPipeline().GetHandle());
            vkCmdDrawIndexed(commandBuffer, numIndices, numInstances, 0, 0, 0);
        }
    }

    void VulkanSwapchain::ScheduleSwapchainFrames(const std::function<void()> &function) const {
        m_engine.GetScheduler().ScheduleFrames(function, NumSwapchainImages());
    }

    Result VulkanSwapchain::CreateSyncObjects() {
        Optional<std::shared_ptr<VulkanFence> > blockingFenceOptional = m_context.CreateFence();
        PETAL_CHECK_OPTIONAL_SILENT(blockingFenceOptional);
        m_blockingCommandFence = blockingFenceOptional.Value();

        Optional<std::vector<std::shared_ptr<VulkanFence> > > fences = m_context.CreateFences(NumSwapchainImages(), VK_FENCE_CREATE_SIGNALED_BIT);
        PETAL_CHECK_OPTIONAL_SILENT(fences);
        m_frameCompleteFences = fences.Value();

        Optional<std::vector<std::shared_ptr<VulkanSemaphore> > > semaphores = m_context.CreateSemaphores(NumSwapchainImages(), 0);
        PETAL_CHECK_OPTIONAL_SILENT(fences);
        m_frameCompleteSemaphores = semaphores.Value();

        semaphores = m_context.CreateSemaphores(NumSwapchainImages(), 0);
        PETAL_CHECK_OPTIONAL_SILENT(fences);
        m_swapchainSemaphores = semaphores.Value();

        return Result::SUCCESS;
    }

    Result VulkanSwapchain::CreateSwapchain(
        const VulkanQueue &queue,
        glm::uvec2 windowSize
    ) {
        Optional<glm::u32> numSwapchainImagesOptional = CheckRequestedNumImagesSupported();
        PETAL_CHECK_OPTIONAL_SILENT(numSwapchainImagesOptional);
        m_numSwapchainImages = numSwapchainImagesOptional.Value();

        Optional<VkPresentModeKHR> presentModeOptional = CheckRequestedPresentMode();
        PETAL_CHECK_OPTIONAL_SILENT(presentModeOptional);

        Result result = ChooseSurfaceFormat();
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        // Create info
        std::array queueFamilies = {queue.GetQueueFamilyIndex()};
        VkSwapchainCreateInfoKHR swapChainCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = m_context.GetSurface(),
            .minImageCount = m_numSwapchainImages,
            .imageFormat = m_swapchainSurfaceFormat.surfaceFormat.format,
            .imageColorSpace = m_swapchainSurfaceFormat.surfaceFormat.colorSpace,
            .imageExtent = VkExtent2D{windowSize.x, windowSize.y},
            .imageArrayLayers = 1,
            .imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = queueFamilies.size(),
            .pQueueFamilyIndices = queueFamilies.data(),
            .preTransform = m_context.GetDevice()->GetSurfaceCapabilities().surfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentModeOptional.Value(),
            .clipped = VK_TRUE
        };

        VkResult res = vkCreateSwapchainKHR(
            m_context.GetDevice()->GetHandle(),
            &swapChainCreateInfo,
            nullptr,
            &m_handle
        );
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_SWAPCHAIN_CREATION_FAILED, m_logger, "Failed to create swapchain: {}", res);

        return Result::SUCCESS;
    }

    Result VulkanSwapchain::CreateSwapchainImages() {
        const auto &device = m_context.GetDevice()->GetHandle();
        // Check the number of swapchain images, it may be more than what we requested
        glm::u32 requestedNumImages = m_numSwapchainImages;
        VkResult res = vkGetSwapchainImagesKHR(
            device,
            m_handle,
            &m_numSwapchainImages,
            nullptr
        );
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_SWAPCHAIN_CREATION_FAILED, m_logger, "Failed to query number of swapchain images: {}", res);
        if (requestedNumImages != m_numSwapchainImages) {
            m_logger->Warn("Requested {} images but vulkan created the swapchain with {}", requestedNumImages, m_numSwapchainImages);
        }

        // Create the images
        m_swapchainTargets = std::make_shared<std::vector<RenderTarget> >(m_numSwapchainImages);

        std::vector<VkImage> images(m_numSwapchainImages);
        res = vkGetSwapchainImagesKHR(device, m_handle, &m_numSwapchainImages, images.data());
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_SWAPCHAIN_CREATION_FAILED, m_logger, "Failed to get the swapchain images");

        for (glm::u32 i = 0; i < m_numSwapchainImages; i++) {
            auto image = std::make_shared<SwapchainImage>();
            image->Name = std::format("Swapchain Image {}", i);

            // Image
            (*m_swapchainTargets)[i].Color = image;
            image->Image = images[i];

            // View
            VkImageViewCreateInfo viewInfo =
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_swapchainSurfaceFormat.surfaceFormat.format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            res = vkCreateImageView(device, &viewInfo, nullptr, &image->View);
            PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_SWAPCHAIN_CREATION_FAILED, m_logger, "Failed to create swapchain image view {} ({})", i, res);
        }

        return Result::SUCCESS;
    }

    Optional<glm::u32> VulkanSwapchain::CheckRequestedNumImagesSupported() {
        const VkSurfaceCapabilitiesKHR &surfaceCapabilities = m_context.GetDevice()->GetSurfaceCapabilities().surfaceCapabilities;
        glm::u32 numImages = m_context.GetGraphicsSettings().NumSwapchainImages;

        // Check maximum
        if (surfaceCapabilities.maxImageCount != VULKAN_SENTINEL_SURFACE_CAPABILITIES_UNLIMITED_SWAPCHAIN_IMAGES && numImages > surfaceCapabilities.maxImageCount) {
            PETAL_CHECK_COND(
                m_context.GetGraphicsSettings().AllowDoubleBufferingFallback,
                Result::VULKAN_SWAPCHAIN_CREATION_FAILED,
                m_logger,
                "Requested {} swapchain images but {} was the maximum and falling back to double buffering was disabled",
                numImages,
                surfaceCapabilities.maxImageCount
            );
            numImages = 2; // fall back to double buffering
            m_logger->Warn("Falling back to double buffering because {} is the maximum number of swapchain images supported by the device", surfaceCapabilities.maxImageCount);
        }

        // Check minimum
        PETAL_CHECK_COND(
            m_context.GetGraphicsSettings().NumSwapchainImages < surfaceCapabilities.minImageCount,
            Result::VULKAN_SWAPCHAIN_CREATION_FAILED,
            m_logger,
            "Requested (or fell back to) {} swapchain images but {} was the minimum",
            numImages,
            surfaceCapabilities.minImageCount
        );

        return numImages;
    }

    Optional<VkPresentModeKHR> VulkanSwapchain::CheckRequestedPresentMode() const {
        for (VkPresentModeKHR requested : m_context.GetGraphicsSettings().PreferredPresentMode) {
            for (VkPresentModeKHR supported : m_context.GetDevice()->GetPresentModes()) {
                if (supported == requested) return requested;
            }
        }
        m_logger->Error("All preferred swapchain present modes are unsupported on the current device");
        return Result::VULKAN_SWAPCHAIN_CREATION_FAILED;
    }

    Result VulkanSwapchain::ChooseSurfaceFormat() {
        auto surfaceFormats = m_context.GetDevice()->GetSurfaceFormats();
        PETAL_CHECK_COND(surfaceFormats.empty(), Result::VULKAN_SWAPCHAIN_CREATION_FAILED, m_logger, "No supported surface formats");

        for (const auto &surfaceFormat : surfaceFormats) {
            if ((surfaceFormat.surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB) &&
                (surfaceFormat.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
                m_swapchainSurfaceFormat = surfaceFormat;
                return Result::SUCCESS;
            }
        }

        m_logger->Warn("Surface format VK_FORMAT_B8G8R8A8_SRGB and VK_COLOR_SPACE_SRGB_NONLINEAR_KHR is not supported, defaulting to format {}", surfaceFormats[0]);
        m_swapchainSurfaceFormat = surfaceFormats[0];
        return Result::SUCCESS;
    }

    Result VulkanSwapchain::CreateDepthBuffer(glm::uvec2 windowSize) {
        Optional<VkFormat> depthFormat = m_context.GetDevice()->FindDepthFormat();
        PETAL_CHECK_OPTIONAL(depthFormat, m_logger, "No supported depth format");

        TextureCreateInfo info = {
            .Size = {windowSize.x, windowSize.y, 1},
            .ImageType = VK_IMAGE_TYPE_2D,
            .ViewType = VK_IMAGE_VIEW_TYPE_2D,
            .Format = depthFormat.Value(),
            .Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .AspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT
        };

        AllocatedOptional<VulkanTexture> texture = m_context.GetMemorySubsystem().CreateTexture("Depth Buffer", info);
        PETAL_CHECK_OPTIONAL(texture, m_logger, "Failed to create depth buffer");

        m_depthBuffer = texture.Release();
        assert(m_swapchainTargets && !m_swapchainTargets->empty());
        for (RenderTarget &target : *m_swapchainTargets) {
            target.Depth = m_depthBuffer;
        }

        return Result::SUCCESS;
    }
} // Petal
