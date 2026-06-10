#include "DeferredRenderer.h"

#include "DeferredGBufferWritePass.h"
#include "DeferredLightingPass.h"
#include "Engine.h"
#include "../../../Assets/Shaders/Common.h"
#include "Graphics/Internal/RenderTarget.h"
#include "Graphics/Internal/VulkanShader.h"
#include "Graphics/Memory/GPUMemorySubsystem.h"
#include "Rendering/Renderer.h"
#include "Rendering/FrameGraph/PresentRenderPass.h"
#include "Rendering/FrameGraph/RenderPass.h"
#include "Timing/Timer.h"

namespace Petal {
    DeferredRenderer::DeferredRenderer(
        Renderer &renderer,
        const std::shared_ptr<Logger> &logger,
        const std::shared_ptr<VulkanShader> &gBufferShader,
        VulkanGraphicsPipeline::PipelineSettings &gBufferPipelineSettings,
        const std::shared_ptr<VulkanShader> &lightingShader,
        VulkanGraphicsPipeline::PipelineSettings &lightingPipelineSettings,
        const glm::vec2 &windowSize,
        const std::vector<RenderTarget> &renderTarget,
        const RenderPassSupplier<DeferredGBufferWritePass> &gBufferWriteSupplier,
        const RenderPassSupplier<DeferredLightingPass> &lightingSupplier,
        Result &resultOut
    ) : m_renderer(renderer),
        m_context(renderer.GetContext()),
        m_logger(logger),
        m_windowSize(windowSize),
        m_gBufferShader(gBufferShader),
        m_lightingShader(lightingShader),
        m_renderTarget(renderTarget) {
        Timer timer;
        m_lightingInfo = std::make_unique<DeferredLighting>();

        resultOut = CreateGBuffer();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreatePipelines(gBufferPipelineSettings, lightingPipelineSettings);
        if (resultOut != Result::SUCCESS) return;

        resultOut = SetupRenderPasses(gBufferWriteSupplier, lightingSupplier);
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateLightingBuffer();
        if (resultOut != Result::SUCCESS) return;

        m_logger->Verbose("Deferred Renderer initialised in {} ms", timer.MillisSinceStart());
    }

    DeferredRenderer::~DeferredRenderer() {
        CancelLightingWriteTasks();
    }

    const std::vector<RenderTarget> &DeferredRenderer::GetGBuffer() const {
        return m_gBuffer;
    }

    const std::vector<RenderTarget> &DeferredRenderer::GetFinalRenderTarget() const {
        return m_renderTarget;
    }

    GraphicsContext &DeferredRenderer::GetContext() const {
        return m_context;
    }

    const std::vector<std::shared_ptr<RenderPass> > &DeferredRenderer::GetPasses() const {
        return m_passes;
    }

    VulkanGraphicsPipeline &DeferredRenderer::GetGBufferPipeline() const {
        return *m_gBufferPipeline;
    }

    VulkanGraphicsPipeline &DeferredRenderer::GetLightingPipeline() const {
        return *m_lightingPipeline;
    }

    Renderer &DeferredRenderer::GetRenderer() const {
        return m_renderer;
    }

    const std::vector<std::shared_ptr<VulkanTexture> > &DeferredRenderer::GetPositionTextures() const {
        return m_gBufferPositionTextures;
    }

    const std::vector<std::shared_ptr<VulkanTexture> > &DeferredRenderer::GetNormalTextures() const {
        return m_gBufferNormalTextures;
    }

    const std::vector<std::shared_ptr<VulkanTexture> > &DeferredRenderer::GetAlbedoTextures() const {
        return m_gBufferAlbedoTextures;
    }

    void DeferredRenderer::SetLighting(const DeferredLighting &lighting) {
        static constexpr auto writeFunc = [](const DeferredRenderer &renderer, glm::u32 index) {
            Result result = renderer.m_context.GetMemorySubsystem().Write((*renderer.m_lightingBuffer)[index], renderer.m_lightingInfo.get(), sizeof(DeferredLighting));
            if (result != Result::SUCCESS) {
                renderer.m_logger->Warn("Failed to write deferred lighting: {}", result);
            }
        };

        CancelLightingWriteTasks();
        *m_lightingInfo.get() = lighting;
        writeFunc(*this, 0); // write this frame

        for (size_t i = 1; i < m_context.GetSwapchain().NumSwapchainImages(); i++) {
            m_context.GetEngine().GetScheduler().ScheduleFramesSyncCancellable([this, i]() { writeFunc(*this, i); }, i);
        }
    }

    Result DeferredRenderer::CreatePipelines(
        VulkanGraphicsPipeline::PipelineSettings &gBufferPipelineSettings,
        VulkanGraphicsPipeline::PipelineSettings &lightingPipelineSettings
    ) {
        // g buffer
        gBufferPipelineSettings.RenderTargets = m_gBuffer;
        AllocatedOptional<VulkanGraphicsPipeline> pipeline = m_gBufferShader->CreatePipeline(gBufferPipelineSettings);
        PETAL_CHECK_OPTIONAL(pipeline, m_logger, "Failed to create deferred G-Buffer pipeline");

        m_gBufferPipeline = pipeline.Release();

        // lighting
        lightingPipelineSettings.RenderTargets = m_renderTarget;
        pipeline = m_lightingShader->CreatePipeline(lightingPipelineSettings);
        PETAL_CHECK_OPTIONAL(pipeline, m_logger, "Failed to create deferred lighting pipeline");

        m_lightingPipeline = pipeline.Release();

        return Result::SUCCESS;
    }

    Result DeferredRenderer::CreateGBuffer() {
        m_gBuffer.clear();
        m_gBuffer.resize(m_context.GetSwapchain().NumSwapchainImages());
        m_gBufferPositionTextures.clear();
        m_gBufferNormalTextures.clear();
        m_gBufferAlbedoTextures.clear();
        m_gBufferSize = 0;

        for (size_t i = 0; i < m_context.GetSwapchain().NumSwapchainImages(); i++) {
            RenderTarget target;

            // Position
            Result result = CreateGBufferColorTexture(
                target,
                std::format("Deferred Position Buffer {}", i),
                VK_FORMAT_R16G16B16A16_SFLOAT,
                m_gBufferPositionTextures
            );
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            // Normal
            result = CreateGBufferColorTexture(
                target,
                std::format("Deferred Normal Buffer {}", i),
                VK_FORMAT_R8G8B8A8_UNORM,
                m_gBufferNormalTextures
            );
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            // Albedo
            result = CreateGBufferColorTexture(
                target,
                std::format("Deferred Albedo Buffer {}", i),
                VK_FORMAT_R8G8B8A8_UNORM,
                m_gBufferAlbedoTextures
            );
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
        VkFormat format,
        std::vector<std::shared_ptr<VulkanTexture> > &pushTo
    ) {
        TextureCreateInfo info = {
            .Size = {m_windowSize.x, m_windowSize.y, 1},
            .ImageType = VK_IMAGE_TYPE_2D,
            .ViewType = VK_IMAGE_VIEW_TYPE_2D,
            .Format = format,
            .Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
        };

        AllocatedOptional<VulkanTexture> textureOptional = m_context.GetMemorySubsystem().CreateTexture(name, info);
        PETAL_CHECK_OPTIONAL(textureOptional, m_logger, "Failed to create {}", name);
        std::shared_ptr<VulkanTexture> texture = textureOptional.Release();

        m_gBufferSize += texture->GetSize();
        target.Colors.push_back({texture, {0, 0, 0, 0}});

        pushTo.push_back(texture);

        return Result::SUCCESS;
    }

    Result DeferredRenderer::SetupRenderPasses(
        const RenderPassSupplier<DeferredGBufferWritePass> &gBufferWriteSupplier,
        const RenderPassSupplier<DeferredLightingPass> &lightingSupplier
    ) {
        PETAL_CHECK_COND(!m_passes.empty(), Result::PETAL_DEFERRED_RENDERING_ERROR, m_logger, "Attempted to setup render passes multiple times");

        // g buffer write pass
        AllocatedOptional<DeferredGBufferWritePass> gBufferWritePass = gBufferWriteSupplier(*this);
        PETAL_CHECK_OPTIONAL(gBufferWritePass, m_logger, "Failed to construct deferred write render pass from the supplier");
        m_passes.push_back(gBufferWritePass.Release());

        // lighting pass
        AllocatedOptional<DeferredLightingPass> lightingPass = lightingSupplier(*this);
        PETAL_CHECK_OPTIONAL(lightingPass, m_logger, "Failed to construct deferred lighting render pass from the supplier");
        m_passes.push_back(lightingPass.Release());

        // present pass
        Result resultOut;
        m_passes.push_back(std::make_shared<PresentRenderPass>(m_logger, "Deferred Present Pass", m_renderTarget, resultOut));
        PETAL_CHECK_COND(resultOut != Result::SUCCESS, resultOut, m_logger, "Failed to construct deferred present pass");

        return Result::SUCCESS;
    }

    Result DeferredRenderer::CreateLightingBuffer() {
        AllocatedOptional<MultipleBuffers> buffer = m_context.GetMemorySubsystem().CreateSwapchainBuffers(
            "Deferred Lighting",
            sizeof(DeferredLighting),
            {.BufferType = BufferType::CONSTANT_BUFFER}
        );

        PETAL_CHECK_OPTIONAL(buffer, m_logger, "Failed to create deferred lighting buffers");

        m_lightingBuffer = buffer.Release();

        Result result = m_lightingShader->BindBuffer("Lighting", m_lightingBuffer->Buffers); // todo don't hard code name
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        SetLighting({
            .Lights = {Light{.Position = {-11.07969, 5.55642, -2.38159}, .Color = {1, 0.5, 0.5}}},
            .NumLights = 1
        });

        return Result::SUCCESS;
    }

    void DeferredRenderer::CancelLightingWriteTasks() {
        for (const Scheduler::SyncTaskId id : m_lightingWriteTasks) {
            m_context.GetEngine().GetScheduler().Cancel<Scheduler::CancellableTaskType::FRAMES_SYNC>(id);
        }
        m_lightingWriteTasks.clear();
    }
} // Petal
