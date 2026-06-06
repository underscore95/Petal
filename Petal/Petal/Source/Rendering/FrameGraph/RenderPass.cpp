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
        const ResourceUsage &usage
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
                .ResourceType = ResourceType::COMBINED_SAMPLER,
                .Usage = usage
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
        assert(action == RenderTargetAction::PRESENT || action == RenderTargetAction::RENDER);
        PETAL_CHECK_COND(
            targets.size() != m_numCommandBuffers,
            Result::PETAL_RENDER_PASS_RESOURCE_ERROR,
            m_logger,
            "TrackRenderTargetPerCommand failed. {} command buffers but {} textures. Pass: {}", m_numCommandBuffers, targets.size(), GetName()
        );

        // COLOR
        bool resizedColors = false; // so we catch error if first 0 images and then others have non 0 images
        std::vector<std::vector<std::shared_ptr<ITexture> > > color;
        // [[albedo0, albedo1, albedo2], [normal0, normal1, normal2]] etc
        for (const RenderTarget &target : targets) {
            // Error check
            if (!resizedColors) {
                color.resize(target.Colors.size());
                resizedColors = true;
            }
            PETAL_CHECK_COND(
                color.size() != target.Colors.size(),
                Result::FRAME_GRAPH_ERROR,
                m_logger,
                "TrackRenderTargetPerCommand - Render Targets had a different number of color textures"
            );

            // Push attachments
            for (size_t i = 0; i < target.Colors.size(); i++) {
                color[i].push_back(target.Colors[i].Texture);
            }
        }

        for (const std::vector<std::shared_ptr<ITexture> > &textures : color) {
            ResourceUsage colorUsage = action == RenderTargetAction::RENDER ? ResourceUsage::ColorAttachment() : ResourceUsage::Present();
            Result result = TrackTexturePerCommand(textures, colorUsage);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
        }

        // DEPTH
        if (action == RenderTargetAction::RENDER) {
            std::vector<std::shared_ptr<ITexture> > depth(targets.size());
            for (size_t i = 0; i < targets.size(); i++) {
                depth[i] = targets[i].Depth;
            }
            Result result = TrackTexturePerCommand(depth, ResourceUsage::DepthAttachmentReadWrite());
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
        }

        return Result::SUCCESS;
    }
} // Petal
