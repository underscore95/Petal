#pragma once

#include "Common.h"
#include "DeferredGBufferWritePass.h"
#include "DeferredLightingPass.h"
#include "Graphics/Internal/VulkanGraphicsPipeline.h"
#include "Rendering/FrameGraph/RenderPass.h"

namespace Petal {
    class SwapchainBuffers;
    struct DeferredLighting;
    class VulkanTexture;
    struct TextureCreateInfo;
    class GraphicsContext;
    struct RenderTarget;
    class Renderer;

    class DeferredRenderer {
        template<typename T>
            requires std::derived_from<T, RenderPass>
        using RenderPassSupplier = std::function<AllocatedOptional<T>(DeferredRenderer &)>;

    public:
        // Pipeline settings will be modified to guarantee compatibility with deferred rendering
        DeferredRenderer(
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
        );

        ~DeferredRenderer();

    public:
        // Call this once per frame
        void Render() const;

        const std::vector<RenderTarget> &GetGBuffer() const;

        // The swapchain or something
        const std::vector<RenderTarget> &GetFinalRenderTarget() const;

        GraphicsContext &GetContext() const;

        const std::vector<std::shared_ptr<RenderPass> > &GetPasses() const;

        VulkanGraphicsPipeline &GetGBufferPipeline() const;

        VulkanGraphicsPipeline &GetLightingPipeline() const;

        Renderer &GetRenderer() const;

        const std::vector<std::shared_ptr<VulkanTexture> > &GetPositionTextures() const;

        const std::vector<std::shared_ptr<VulkanTexture> > &GetNormalTextures() const;

        const std::vector<std::shared_ptr<VulkanTexture> > &GetAlbedoTextures() const;

        void SetLighting(const std::shared_ptr<DeferredLighting> &lighting);

    private:
        Result CreatePipelines(
            VulkanGraphicsPipeline::PipelineSettings &gBufferPipelineSettings,
            VulkanGraphicsPipeline::PipelineSettings &lightingPipelineSettings
        );

        Result CreateGBuffer();

        Result CreateGBufferColorTexture(
            RenderTarget &target,
            const std::string &name,
            VkFormat format,
            std::vector<std::shared_ptr<VulkanTexture> > &pushTo
        );

        Result SetupRenderPasses(
            const RenderPassSupplier<DeferredGBufferWritePass> &gBufferWriteSupplier,
            const RenderPassSupplier<DeferredLightingPass> &lightingSupplier
        );

        Result CreateLightingBuffer();

    private:
        Renderer &m_renderer;
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        glm::vec2 m_windowSize;
        std::shared_ptr<VulkanShader> m_gBufferShader;
        std::shared_ptr<VulkanShader> m_lightingShader;
        std::unique_ptr<VulkanGraphicsPipeline> m_gBufferPipeline;
        std::unique_ptr<VulkanGraphicsPipeline> m_lightingPipeline;
        std::vector<RenderTarget> m_gBuffer;
        std::vector<std::shared_ptr<VulkanTexture> > m_gBufferAlbedoTextures;
        std::vector<std::shared_ptr<VulkanTexture> > m_gBufferNormalTextures;
        std::vector<std::shared_ptr<VulkanTexture> > m_gBufferPositionTextures;
        size_t m_gBufferSize = 0;
        std::vector<std::shared_ptr<RenderPass> > m_passes;
        std::vector<RenderTarget> m_renderTarget;
        std::unique_ptr<SwapchainBuffers> m_lightingBuffer;
    };
} // Petal
