#include "PresentRenderPass.h"
#include "Graphics/Internal/RenderTarget.h"

namespace Petal {
    PresentRenderPass::PresentRenderPass(
        const std::shared_ptr<Logger> &logger,
        std::string name,
        const std::vector<RenderTarget> &targets,
        Result &resultOut
    ) : RenderPass(logger, name, targets.size()),
        m_targets(targets) {
        resultOut = TrackRenderTargetPerCommand(targets, RenderTargetAction::PRESENT);
    }

    Result PresentRenderPass::Record(const std::shared_ptr<CommandBufferVector> &commands) const {
        return Result::SUCCESS;
    }
} // Petal
