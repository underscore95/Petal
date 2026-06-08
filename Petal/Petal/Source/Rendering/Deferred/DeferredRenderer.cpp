#include "DeferredRenderer.h"

#include "DeferredGBufferWritePass.h"
#include "Graphics/Internal/RenderTarget.h"
#include "Graphics/Memory/GPUMemorySubsystem.h"
#include "Rendering/Renderer.h"
#include "Rendering/FrameGraph/PresentRenderPass.h"
#include "Rendering/FrameGraph/RenderPass.h"
#include "Timing/Timer.h"

namespace Petal {
    DeferredRenderer::DeferredRenderer(
        Renderer &renderer,
        const std::shared_ptr<Logger> &logger,
        const glm::vec2 &windowSize,
        const RenderPassSupplier &renderPassSupplier,
        Result &resultOut
    ) : m_renderer(renderer),
        m_context(renderer.GetContext()),
        m_logger(logger),
        m_windowSize(windowSize) {
        Timer timer;

        resultOut = CreateGBuffer();
        if (resultOut != Result::SUCCESS) return;

        resultOut = SetupRenderPasses(renderPassSupplier);
        if (resultOut != Result::SUCCESS) return;

        m_logger->Verbose("Deferred Renderer initialised in {} ms", timer.MillisSinceStart());
    }

    const std::vector<RenderTarget> &DeferredRenderer::GetGBuffer() const {
        return m_gBuffer;
    }

    GraphicsContext &DeferredRenderer::GetContext() const {
        return m_context;
    }

    const std::vector<std::shared_ptr<RenderPass>> &DeferredRenderer::GetPasses() const {
        return m_passes;
    }

    Result DeferredRenderer::CreateGBuffer() {
        m_gBuffer.clear();
        m_gBuffer.resize(m_context.GetSwapchain().NumSwapchainImages());
        m_gBufferSize = 0;

        for (size_t i = 0; i < m_context.GetSwapchain().NumSwapchainImages(); i++) {
            RenderTarget target;

            // Position
            Result result = CreateGBufferColorTexture(target, std::format("Deferred Position Buffer {}", i), VK_FORMAT_R16G16B16A16_SFLOAT);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            // Normal
            result = CreateGBufferColorTexture(target, std::format("Deferred Normal Buffer {}", i), VK_FORMAT_A2B10G10R10_UNORM_PACK32);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            // Albedo
            result = CreateGBufferColorTexture(target, std::format("Deferred Albedo Buffer {}", i), VK_FORMAT_R8G8B8A8_UNORM);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            // Depth buffer
            Optional<VkFormat> depthFormat = m_context.GetDevice()->FindDepthFormat();
            PETAL_CHECK_OPTIONAL(depthFormat, m_logger, "No supported depth format");

            TextureCreateInfo info = {
                .Size = {m_windowSize.x, m_windowSize.y, 1},
                .ImageType = VK_IMAGE_TYPE_2D,
                .ViewType = VK_IMAGE_VIEW_TYPE_2D,
                .Format = depthFormat.Value(),
                .Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .AspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT
            };

            AllocatedOptional<VulkanTexture> texture = m_context.GetMemorySubsystem().CreateTexture(std::format("Deferred Depth Buffer {}", i), info);
            PETAL_CHECK_OPTIONAL(texture, m_logger, "Failed to create depth buffer");

            m_gBufferSize += texture->GetSize();
            target.Depth = texture.Release();

            m_gBuffer[i] = target;
        }

        m_logger->Verbose("Allocated {} MB G-Buffer for Deferred Rendering", m_gBufferSize / 1024 / 1024);

        return Result::SUCCESS;
    }

    Result DeferredRenderer::CreateGBufferColorTexture(
        RenderTarget &target,
        const std::string &name,
        VkFormat format
    ) {
        TextureCreateInfo info = {
            .Size = {m_windowSize.x, m_windowSize.y, 1},
            .ImageType = VK_IMAGE_TYPE_2D,
            .ViewType = VK_IMAGE_VIEW_TYPE_2D,
            .Format = format,
            .Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
        };

        AllocatedOptional<VulkanTexture> texture = m_context.GetMemorySubsystem().CreateTexture(name, info);
        PETAL_CHECK_OPTIONAL(texture, m_logger, "Failed to create {}", name);

        m_gBufferSize += texture->GetSize();
        target.Colors.push_back({texture.Release(), {0, 0, 0, 0}});

        return Result::SUCCESS;
    }

    Result DeferredRenderer::SetupRenderPasses(const RenderPassSupplier &renderPassSupplier) {
        PETAL_CHECK_COND(!m_passes.empty(), Result::PETAL_DEFERRED_RENDERING_ERROR, m_logger, "Attempted to setup render passes multiple times");

        std::unique_ptr<RenderPass> writePass = renderPassSupplier(*this);
        PETAL_CHECK_COND(!m_passes.empty(), Result::PETAL_DEFERRED_RENDERING_ERROR, m_logger, "Failed to construct deferred write render pass from the supplier");

        m_passes.push_back(std::move(writePass));

        Result resultOut;
        m_passes.push_back(std::make_shared<PresentRenderPass>(m_logger, "Deferred Present Pass", m_gBuffer, resultOut));
        PETAL_CHECK_COND(resultOut != Result::SUCCESS, resultOut, m_logger, "Failed to construct deferred present pass");

        return Result::SUCCESS;
    }
} // Petal
