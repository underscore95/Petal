#include "RenderPass.h"
#include "Graphics/Internal/ITexture.h"
#include "Graphics/Internal/RenderTarget.h"

namespace Petal {
    constexpr glm::u32 MAX_COMMAND_BUFFERS = 128;

    RenderPass::RenderPass(
        const std::shared_ptr<Logger> &logger,
        glm::u32 numCommandBuffers
    )
        : m_logger(logger),
          m_numCommandBuffers(numCommandBuffers) {
        assert(m_numCommandBuffers < MAX_COMMAND_BUFFERS); // since we allocate an vector for every buffer, don't want to acccidentally allocate a billion vectors

        m_accessedResources.resize(m_numCommandBuffers);
    }

    const std::vector<std::vector<RenderPass::PassResource> > &RenderPass::GetAccessedResources() const {
        return m_accessedResources;
    }

    size_t RenderPass::GetNumCommandBuffers() const {
        return m_numCommandBuffers;
    }

    Result RenderPass::TrackResource(const PassResource &resource, size_t index) {
        PETAL_CHECK_COND(
            index >= m_numCommandBuffers,
            Result::PETAL_RENDER_PASS_RESOURCE_ERROR,
            m_logger,
            "Index {} (num command buffers: {}) out of bounds when tracking resource {} in pass {}", index, m_numCommandBuffers, resource.Resource->GetName(), GetName()
        );
        m_accessedResources[index].push_back(resource);
        return Result::SUCCESS;
    }

    Result RenderPass::TrackResourceFully(const PassResource &resource) {
        for (size_t i = 0; i < m_numCommandBuffers; i++) {
            Result result = TrackResource(resource, i);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
        }
        return Result::SUCCESS;
    }

    Result RenderPass::TrackTexturePerCommand(
        const std::vector<std::shared_ptr<ITexture> > &textures,
        ResourceAccess accessType,
        VkImageLayout requiredLayout
    ) {
        PETAL_CHECK_COND(
            textures.size() != m_numCommandBuffers,
            Result::PETAL_RENDER_PASS_RESOURCE_ERROR,
            m_logger,
            "TrackTexturePerCommand failed. {} command buffers but {} textures. Pass: {}. Texture names: {}", m_numCommandBuffers, textures.size(), GetName(),
            INameable::ToNameVector(textures)
        );

        for (size_t i = 0; i < textures.size(); i++) {
            PassResource resource = {
                .Resource = textures[i],
                .AccessType = accessType,
                .RequiredImageLayout = requiredLayout,
                .ResourceType = ResourceType::COMBINED_SAMPLER
            };
            Result result = TrackResource(resource, i);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
        }

        return Result::SUCCESS;
    }

    Result RenderPass::TrackRenderTargetPerCommand(
        const std::vector<RenderTarget> &targets,
        RenderTargetAction action
    ) {
        PETAL_CHECK_COND(
            targets.size() != m_numCommandBuffers,
            Result::PETAL_RENDER_PASS_RESOURCE_ERROR,
            m_logger,
            "TrackRenderTargetPerCommand failed. {} command buffers but {} textures. Pass: {}", m_numCommandBuffers, targets.size(), GetName()
        );

        // COLOR
        constexpr std::array COLOR_LAYOUTS = {
            VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
        std::vector<std::shared_ptr<ITexture> > color(targets.size());
        for (size_t i = 0; i < targets.size(); i++) {
            color[i] = targets[i].Color;
        }
        ResourceAccess colorAccess;
        if (action == RenderTargetAction::PRESENT) colorAccess = ResourceAccess::READ;
        else if (action == RenderTargetAction::RENDER) colorAccess = ResourceAccess::WRITE;
        else
            PETAL_ERROR(Result::FRAME_GRAPH_ERROR, m_logger, "Unsupported RenderTargetAction {}", colorAccess);
        Result result = TrackTexturePerCommand(color, colorAccess, COLOR_LAYOUTS[static_cast<size_t>(action)]);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        // DEPTH
        std::vector<std::shared_ptr<ITexture> > depth(targets.size());
        for (size_t i = 0; i < targets.size(); i++) {
            depth[i] = targets[i].Depth;
        }
        // todo access flags VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT
        result = TrackTexturePerCommand(depth, ResourceAccess::READ_WRITE, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        return Result::SUCCESS;
    }
} // Petal
