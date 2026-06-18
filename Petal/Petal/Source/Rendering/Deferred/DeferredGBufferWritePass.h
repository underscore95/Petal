#pragma once

#include "Graphics/Internal/VulkanSwapchain.h"
#include "Rendering/FrameGraph/RenderPass.h"

namespace Petal {
    class DeferredRenderer;

    class DeferredGBufferWritePass : public RenderPass {
    public:
        DeferredGBufferWritePass(
            const std::shared_ptr<Logger> &logger,
            const std::string& name,
            glm::u32 numCommandBuffers,
            DeferredRenderer &deferredRenderer
        );

    public:
        virtual Result Record(const std::shared_ptr<VulkanSwapchain::RenderCommandBuffers> &renderCommands) const = 0;

    private:
        Result Record(const std::shared_ptr<CommandBufferVector> &commands) const final;

    private:
        DeferredRenderer &m_deferredRenderer;
    };
} // Petal
