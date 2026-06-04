#include "RenderPass.h"

namespace Petal {
    RenderPass::RenderPass(
        const std::shared_ptr<Logger> &logger,
        const std::vector<PassResource> &resources
    )
        : m_logger(logger),
          m_resources(resources) {
    }

    const std::vector<RenderPass::PassResource> &RenderPass::GetAccessedResources() const {
        return m_resources;
    }

    const std::string &RenderPass::GetName() const {
        return m_name;
    }
} // Petal
