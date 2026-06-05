#pragma once

#include "Common.h"
#include "RenderPass.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Memory/IVulkanResource.h"

namespace Petal {
    class CommandBufferVector;
    class RenderPass;

    class FrameGraph {
        struct ResourceState {
            const RenderPass *LastPass;
            ResourceUsage LastUsage;

            ResourceState();
            ResourceState(const RenderPass &lastPass, const ResourceUsage &lastUsage);
        };

    public:
        FrameGraph(
            GraphicsContext &context,
            const std::shared_ptr<Logger> &logger,
            const std::shared_ptr<CommandBufferVector> &commands,
            std::vector<std::unique_ptr<RenderPass> > passes,
            Result &resultOut
        );

    public:
        Result RerecordCommandBuffers();

        const std::vector<std::string> &ToString() const;

    private:
        Result PushBarriers(
            Optional<ResourceState> &state,
            const RenderPass::PassResource &resource,
            std::vector<VkBufferMemoryBarrier2> &bufferBarriers,
            std::vector<VkImageMemoryBarrier2> &imageBarriers,
            std::string &graphVisualRepresentation
        );

        Result Record();

    private:
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        std::shared_ptr<CommandBufferVector> m_commands;
        std::unordered_map<std::shared_ptr<IVulkanResource>, Optional<ResourceState> > m_resources;
        std::vector<std::unique_ptr<RenderPass> > m_passes;
        std::vector<std::string> m_visualRepresentation;
    };
} // Petal
