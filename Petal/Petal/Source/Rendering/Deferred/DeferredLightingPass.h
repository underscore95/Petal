#pragma once
#include "Graphics/Internal/VulkanSwapchain.h"
#include "Rendering/FrameGraph/RenderPass.h"

namespace Petal {
    class MeshResource;
    class DeferredRenderer;

    class DeferredLightingPass : public RenderPass {
    public:
        DeferredLightingPass(
            const std::shared_ptr<Logger> &logger,
            const std::string &name,
            glm::u32 numCommandBuffers,
            DeferredRenderer &deferredRenderer,
            Result &resultOut
        );

    public:
        Result Record(const std::shared_ptr<CommandBufferVector> &commands) const;

    private:

        Result CreateQuadMesh();

    protected:
        std::unique_ptr<MeshResource> m_quadMesh;

    private:
        DeferredRenderer &m_deferredRenderer;
    };
} // Petal
