#pragma once

#include "Common.h"
#include "RenderPass.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Memory/IVulkanResource.h"
#include "Graphics/Resources/ResourceAccess.h"

namespace Petal {
    class CommandBufferVector;
    class RenderPass;

    class FrameGraph {
        struct ResourceState {
            std::reference_wrapper<const RenderPass> LastPass;
            ResourceAccess LastAccess;
            Optional<VkImageLayout> LastImageLayout;
        };

    public:
        FrameGraph(
            GraphicsContext& context,
            const std::shared_ptr<Logger> &logger,
            const std::shared_ptr<CommandBufferVector> &commands,
            const std::vector<std::shared_ptr<IVulkanResource> > &resources,
            std::forward_list<RenderPass> &&passes,
            Result &resultOut
        );

    public:
        Result RerecordCommandBuffers();

    private:
        Result PushBarriers(
            const ResourceState &state,
            const RenderPass::PassResource &resource,
            std::vector<VkBufferMemoryBarrier2> &bufferBarriers,
            std::vector<VkImageMemoryBarrier2> &imageBarriers
        );

        Result Record();

    private:
        GraphicsContext& m_context;
        std::shared_ptr<Logger> m_logger;
        std::shared_ptr<CommandBufferVector> m_commands;
        std::unordered_map<std::shared_ptr<IVulkanResource>, Optional<ResourceState> > m_resources;
        std::forward_list<RenderPass> m_passes;
    };
} // Petal
