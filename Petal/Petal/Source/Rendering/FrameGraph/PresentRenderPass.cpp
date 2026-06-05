#include "PresentRenderPass.h"
#include "Graphics/Internal/RenderTarget.h"

namespace Petal {
    PresentRenderPass::PresentRenderPass(
        const std::shared_ptr<Logger> &logger,
        std::string name,
        const std::vector<RenderTarget> &targets,
        Result &resultOut
    ) : RenderPass(logger, targets.size()),
        m_name(name),
        m_targets(targets) {
        resultOut = TrackRenderTargetPerCommand(targets, RenderTargetAction::PRESENT);
    }

    const std::string &PresentRenderPass::GetName() const {
        return m_name;
    }

    Result PresentRenderPass::Record(const std::shared_ptr<CommandBufferVector> &commands) const {
        return Result::SUCCESS;
    }
} // Petal
