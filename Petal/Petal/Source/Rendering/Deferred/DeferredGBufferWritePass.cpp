#include "DeferredGBufferWritePass.h"

#include "DeferredRenderer.h"
#include "Graphics/GraphicsContext.h"

namespace Petal {
    DeferredGBufferWritePass::DeferredGBufferWritePass(
        const std::shared_ptr<Logger> &logger,
        const std::string& name,
        glm::u32 numCommandBuffers,
        DeferredRenderer &deferredRenderer
    ) : RenderPass(logger, name, numCommandBuffers),
        m_deferredRenderer(deferredRenderer) {
        TrackRenderTargetPerCommand(m_deferredRenderer.GetGBuffer(), ResourceUsage::ColorAttachment(), ResourceUsage::DepthAttachmentReadWrite());
    }

    Result DeferredGBufferWritePass::Record(const std::shared_ptr<CommandBufferVector> &commands) const {
        std::shared_ptr<VulkanSwapchain::RenderCommandBuffers> renderCommands = m_deferredRenderer.GetContext().GetSwapchain().CmdBeginRendering(
            commands,
            m_deferredRenderer.GetGBuffer()
        );
        return Record(renderCommands);
    }
} // Petal
