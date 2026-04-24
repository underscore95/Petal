#include "VulkanSwapchain.h"
#include "Rendering/Renderer.h"
#include "Engine.h"
#include "RenderingDevice.h"
#include "VulkanQueue.h"
#include "AppInfo/AppInfo.h"
#include "AppInfo/AppInfo.h"
#include "CommandBuffers/CommandBufferVector.h"
#include "Sync/VulkanFence.h"
#include "Sync/VulkanSemaphore.h"
#include "Window/Window.h"

namespace Petal {
    VulkanSwapchain::VulkanSwapchain(
        Engine &engine,
        Renderer &renderer,
        const VulkanQueue &queueFamily,
        Result &resultOut
    ) : m_renderer(renderer) {
        m_logger = engine.GetLoggerSystem().GetLogger(LoggerSystem::RENDERING_LOGGER);

        glm::uvec2 windowSize = renderer.GetWindow().GetDimensions();
        resultOut = CreateSwapchain(queueFamily, windowSize);
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateSwapchainImages();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateSyncObjects();
        if (resultOut != Result::SUCCESS) return;

        AllocatedOptional<CommandBufferVector> commands = CreateImageTransitionCommands(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        if (commands.IsEmpty()) {
            resultOut = commands.GetResult();
            return;
        }
        m_beginRenderingCommands = commands.Release();

        commands = CreateImageTransitionCommands(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        if (commands.IsEmpty()) {
            resultOut = commands.GetResult();
            return;
        }
        m_endRenderingCommands = commands.Release();

        m_logger->Verbose("Created swapchain for window size {}x{}", windowSize.x, windowSize.y);
    }

    VulkanSwapchain::~VulkanSwapchain() {
        for (auto &imageView : m_imageViews) {
            vkDestroyImageView(m_renderer.GetDevice()->GetDevice(), imageView, nullptr);
        }
        m_imageViews.clear();

        // Destroying the swapchain destroys the images
        vkDestroySwapchainKHR(
            m_renderer.GetDevice()->GetDevice(),
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
        VkResult res = vkWaitForFences(m_renderer.GetDevice()->GetDevice(), 1, &frameCompleteFence, true, PETAL_U64_MAX);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_BEGIN_RENDER_FAILED, m_logger, "Failed to wait for fence: {}", res);

        res = vkResetFences(m_renderer.GetDevice()->GetDevice(), 1, &frameCompleteFence);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_BEGIN_RENDER_FAILED, m_logger, "Failed to reset fence: {}", res);

        // Request new frame
        res = vkAcquireNextImageKHR(
            m_renderer.GetDevice()->GetDevice(),
            m_handle,
            PETAL_U64_MAX,
            m_swapchainSemaphores[m_swapchainIndex]->GetHandle(),
            VK_NULL_HANDLE,
            &m_swapchainIndex
        );
        PETAL_CHECK_COND_SILENT(res == VK_ERROR_OUT_OF_DATE_KHR, Result::PETAL_WINDOW_RESIZED);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_BEGIN_RENDER_FAILED, m_logger, "Failed acquire image with current swapchain index {}: {}", m_swapchainIndex, res);

        SubmitFrameCommand(CommandBufferStrongRef(m_beginRenderingCommands, m_swapchainIndex));

        return Result::SUCCESS;
    }

    Result VulkanSwapchain::EndRendering() {
        SubmitFrameCommand(CommandBufferStrongRef(m_endRenderingCommands, m_swapchainIndex));

        VkSemaphoreSubmitInfo waitInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = m_swapchainSemaphores[GetSwapchainIndex()]->GetHandle(),
            .value = 1,
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
            .deviceIndex = 0
        };

        VkSemaphoreSubmitInfo signalInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = m_frameCompleteSemaphores[GetSwapchainIndex()]->GetHandle(),
            .value = 1,
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
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
            m_renderer.GetDevice()->GetGraphicsQueueFamily().GetHandle(),
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

        VkResult res = vkQueuePresentKHR(m_renderer.GetDevice()->GetGraphicsQueueFamily().GetHandle(), &presentInfo);
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

    void VulkanSwapchain::CmdClear(
        VkCommandBuffer commandBuffer,
        const Color &color,
        glm::u32 swapchainIndex
    ) const {
        VkImageSubresourceRange range = Renderer::DEFAULT_IMAGE_SUBRESOURCE_RANGE;
        VkClearColorValue colorValue;
        color.WriteFloats(colorValue.float32);
        vkCmdClearColorImage(
            commandBuffer,
            m_images[swapchainIndex],
            VK_IMAGE_LAYOUT_GENERAL,
            &colorValue,
            1,
            &range
        );
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

    Result VulkanSwapchain::CreateSyncObjects() {
        Optional<std::vector<std::shared_ptr<VulkanFence> > > fences = m_renderer.CreateFences(NumSwapchainImages(), VK_FENCE_CREATE_SIGNALED_BIT);
        PETAL_CHECK_OPTIONAL_SILENT(fences);
        m_frameCompleteFences = *fences.Value();

        Optional<std::vector<std::shared_ptr<VulkanSemaphore> > > semaphores = m_renderer.CreateSemaphores(NumSwapchainImages(), 0);
        PETAL_CHECK_OPTIONAL_SILENT(fences);
        m_frameCompleteSemaphores = *semaphores.Value();

        semaphores = m_renderer.CreateSemaphores(NumSwapchainImages(), 0);
        PETAL_CHECK_OPTIONAL_SILENT(fences);
        m_swapchainSemaphores = *semaphores.Value();

        return Result::SUCCESS;
    }

    Result VulkanSwapchain::CreateSwapchain(
        const VulkanQueue &queue,
        glm::uvec2 windowSize
    ) {
        Optional<glm::u32> numSwapchainImagesOptional = CheckRequestedNumImagesSupported();
        PETAL_CHECK_OPTIONAL_SILENT(numSwapchainImagesOptional);
        m_numSwapchainImages = *numSwapchainImagesOptional.Value();

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
            .surface = m_renderer.GetSurface(),
            .minImageCount = m_numSwapchainImages,
            .imageFormat = m_swapchainSurfaceFormat.surfaceFormat.format,
            .imageColorSpace = m_swapchainSurfaceFormat.surfaceFormat.colorSpace,
            .imageExtent = VkExtent2D{windowSize.x, windowSize.y},
            .imageArrayLayers = 1,
            .imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = queueFamilies.size(),
            .pQueueFamilyIndices = queueFamilies.data(),
            .preTransform = m_renderer.GetDevice()->GetSurfaceCapabilities().surfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = *presentModeOptional.Value(),
            .clipped = VK_TRUE
        };

        VkResult res = vkCreateSwapchainKHR(
            m_renderer.GetDevice()->GetDevice(),
            &swapChainCreateInfo,
            nullptr,
            &m_handle
        );
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_SWAPCHAIN_CREATION_FAILED, m_logger, "Failed to create swapchain: {}", res);

        return Result::SUCCESS;
    }

    Result VulkanSwapchain::CreateSwapchainImages() {
        const auto &device = m_renderer.GetDevice()->GetDevice();
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
        m_images.resize(m_numSwapchainImages);
        m_imageViews.resize(m_numSwapchainImages);

        res = vkGetSwapchainImagesKHR(device, m_handle, &m_numSwapchainImages, m_images.data());
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_SWAPCHAIN_CREATION_FAILED, m_logger, "Failed to get the swapchain images");

        for (glm::u32 i = 0; i < m_numSwapchainImages; i++) {
            VkImageViewCreateInfo viewInfo =
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = m_images[i],
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

            res = vkCreateImageView(device, &viewInfo, nullptr, &m_imageViews[i]);
            PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_SWAPCHAIN_CREATION_FAILED, m_logger, "Failed to create swapchain image view {} ({})", i, res);
        }

        return Result::SUCCESS;
    }

    Optional<glm::u32> VulkanSwapchain::CheckRequestedNumImagesSupported() {
        const VkSurfaceCapabilitiesKHR &surfaceCapabilities = m_renderer.GetDevice()->GetSurfaceCapabilities().surfaceCapabilities;
        glm::u32 numImages = m_renderer.GetRenderSettings().NumSwapchainImages;

        // Check maximum
        if (surfaceCapabilities.maxImageCount != VULKAN_SENTINEL_SURFACE_CAPABILITIES_UNLIMITED_SWAPCHAIN_IMAGES && numImages > surfaceCapabilities.maxImageCount) {
            PETAL_CHECK_COND(
                m_renderer.GetRenderSettings().AllowDoubleBufferingFallback,
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
            m_renderer.GetRenderSettings().NumSwapchainImages < surfaceCapabilities.minImageCount,
            Result::VULKAN_SWAPCHAIN_CREATION_FAILED,
            m_logger,
            "Requested (or fell back to) {} swapchain images but {} was the minimum",
            numImages,
            surfaceCapabilities.minImageCount
        );

        return numImages;
    }

    Optional<VkPresentModeKHR> VulkanSwapchain::CheckRequestedPresentMode() const {
        for (VkPresentModeKHR requested : m_renderer.GetRenderSettings().PreferredPresentMode) {
            for (VkPresentModeKHR supported : m_renderer.GetDevice()->GetPresentModes()) {
                if (supported == requested) return requested;
            }
        }
        m_logger->Error("All preferred swapchain present modes are unsupported on the current device");
        return Result::VULKAN_SWAPCHAIN_CREATION_FAILED;
    }

    Result VulkanSwapchain::ChooseSurfaceFormat() {
        auto surfaceFormats = m_renderer.GetDevice()->GetSurfaceFormats();
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

    AllocatedOptional<CommandBufferVector> VulkanSwapchain::CreateImageTransitionCommands(
        VkImageLayout oldLayout,
        VkImageLayout newLayout
    ) {
        AllocatedOptional<CommandBufferVector> commandsOptional = m_renderer.CreateCommandBuffers(
            m_renderer.GetDevice()->GetGraphicsQueueFamily(),
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            m_numSwapchainImages
        );
        PETAL_CHECK_OPTIONAL_SILENT(commandsOptional);

        for (glm::u32 i = 0; i < commandsOptional.Value()->Size(); i++) {
            Result result = commandsOptional.Value()->Begin(i, 0);
            PETAL_CHECK_COND(result != Result::SUCCESS, result, m_logger, "");

            m_renderer.CmdTransitionImage(
                commandsOptional.Value()->GetHandle(i),
                m_images[i],
                oldLayout,
                newLayout
            );

            result = commandsOptional.Value()->End(i);
            PETAL_CHECK_COND(result != Result::SUCCESS, result, m_logger, "");
        }

        return std::move(commandsOptional);
    }
} // Petal
