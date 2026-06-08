#pragma once

#include "Common.h"

namespace Petal {
    class RenderPass;
}

namespace Petal {
    class DeferredGBufferWritePass;
}

namespace Petal {
    class VulkanTexture;
    struct TextureCreateInfo;
    class GraphicsContext;
    struct RenderTarget;
    class Renderer;

    class DeferredRenderer {
        using RenderPassSupplier = std::function<std::unique_ptr<DeferredGBufferWritePass>(DeferredRenderer &)>;

    public:
        DeferredRenderer(
            Renderer &renderer,
            const std::shared_ptr<Logger> &logger,
            const glm::vec2 &windowSize,
            const RenderPassSupplier &renderPassSupplier,
            Result &resultOut
        );

    public:
        const std::vector<RenderTarget> &GetGBuffer() const;

        GraphicsContext &GetContext() const;

        const std::vector<std::shared_ptr<RenderPass>> &GetPasses() const;

    private:
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
        std::vector<RenderTarget> m_gBuffer;
        std::vector<std::shared_ptr<RenderPass> > m_passes;
        size_t m_gBufferSize = 0;
    };
} // Petal
