#pragma once

#include "Common.h"
#include "Graphics/Internal/VulkanGraphicsPipeline.h"

namespace Petal {
    class RenderPass;
    class DeferredGBufferWritePass;
    class VulkanTexture;
    struct TextureCreateInfo;
    class GraphicsContext;
    struct RenderTarget;
    class Renderer;

    class DeferredRenderer {
        using RenderPassSupplier = std::function<std::unique_ptr<DeferredGBufferWritePass>(DeferredRenderer &)>;

    public:
        // Pipeline settings will be modified to guarantee compatibility with deferred rendering
        DeferredRenderer(
            Renderer &renderer,
            const std::shared_ptr<Logger> &logger,
            const std::shared_ptr<VulkanShader> &shader,
            VulkanGraphicsPipeline::PipelineSettings &pipelineSettings,
            const glm::vec2 &windowSize,
            const RenderPassSupplier &renderPassSupplier,
            Result &resultOut
        );

    public:
        const std::vector<RenderTarget> &GetGBuffer() const;

        GraphicsContext &GetContext() const;

        const std::vector<std::shared_ptr<RenderPass> > &GetPasses() const;

        VulkanGraphicsPipeline & GetPipeline() const;

    private:
        Result CreatePipeline(VulkanGraphicsPipeline::PipelineSettings &settings);

        Result CreateGBuffer();

        Result CreateGBufferColorTexture(
            RenderTarget &target,
            const std::string &name,
            VkFormat format
        );

        Result SetupRenderPasses(const RenderPassSupplier &renderPassSupplier);

    private:
        Renderer &m_renderer;
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        glm::vec2 m_windowSize;
        std::shared_ptr<VulkanShader> m_shader;
        std::unique_ptr<VulkanGraphicsPipeline> m_pipeline;
        std::vector<RenderTarget> m_gBuffer;
        std::vector<std::shared_ptr<RenderPass> > m_passes;
        size_t m_gBufferSize = 0;
    };
} // Petal
