#pragma once

#include "RenderPass.h"

namespace Petal {
    // RenderPass which does nothing other than transition render targets to present optimal
    class PresentRenderPass final : public RenderPass {
    public:
        PresentRenderPass(
            const std::shared_ptr<Logger> &logger,
            std::string name,
            const std::vector<RenderTarget> &targets,
            Result &resultOut
        );

    public:
        const std::string &GetName() const override;

        Result Record(const std::shared_ptr<CommandBufferVector> &commands) const override;

    private:
        std::string m_name;
        std::vector<RenderTarget> m_targets;
    };
} // Petal
