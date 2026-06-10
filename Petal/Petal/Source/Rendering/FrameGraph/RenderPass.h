#pragma once

#include "INameable.h"
#include "ResourceUsage.h"
#include "Graphics/Memory/IVulkanResource.h"
#include "Graphics/Resources/ResourceType.h"

namespace Petal {
    struct RenderTarget;
    class ITexture;
    class VulkanTexture;
    class VulkanBuffer;
    class CommandBufferVector;

    class RenderPass : public INameable {
    public:
        struct PassResource {
            std::shared_ptr<IVulkanResource> Resource;
            ResourceType ResourceType;
            ResourceUsage Usage;

            friend bool operator==(const PassResource &lhs, const PassResource &rhs) {
                return lhs.Resource == rhs.Resource;
            }

            friend bool operator!=(const PassResource &lhs, const PassResource &rhs) {
                return !(lhs == rhs);
            }
        };

    public:
        RenderPass(
            const std::shared_ptr<Logger> &logger,
            const std::string &name,
            glm::u32 numCommandBuffers
        );

    public:
        const std::string &GetName() const override;

        const std::vector<std::vector<PassResource> > &GetAccessedResources() const;

        // Record the commands of this render pass
        virtual Result Record(const std::shared_ptr<CommandBufferVector> &commands) const = 0;

        size_t GetNumCommandBuffers() const;

    protected:
        // Track a resource, you should only call this in the constructor

        // A single command buffer uses this resource (or multiple; call once per buffer)
        Result TrackResource(const PassResource &resource, size_t index);

        // Every command buffer uses this resource
        Result TrackResourceFully(const PassResource &resource);

        Result TrackTexturePerCommand(
            const std::vector<std::shared_ptr<ITexture> > &textures,
            const ResourceUsage &usage
        );

        Result TrackRenderTargetPerCommand(
            const std::vector<RenderTarget> &targets,
            const ResourceUsage &colorUsage,
            const ResourceUsage &depthUsage
        );

    protected:
        std::shared_ptr<Logger> m_logger;

    private:
        std::string m_name;
        // each command buffer accesses std::vector<PassResource>
        std::vector<std::vector<PassResource> > m_accessedResources;
        size_t m_numCommandBuffers;
    };
} // Petal
