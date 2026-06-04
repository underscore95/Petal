#pragma once

#include "INameable.h"
#include "Graphics/Memory/IVulkanResource.h"
#include "Graphics/Resources/ResourceAccess.h"
#include "Graphics/Resources/ResourceType.h"

namespace Petal {
    class VulkanTexture;
    class VulkanBuffer;
    class CommandBufferVector;

    class RenderPass : public INameable {
    public:
        struct PassResource {
            std::shared_ptr<IVulkanResource> Resource;
            ResourceAccess AccessType;
            Optional<VkImageLayout> RequiredImageLayout;
            ResourceType ResourceType;
        };

    public:
        RenderPass(
            const std::shared_ptr<Logger> &logger,
            const std::vector<PassResource> &resources
        );

    public:
        const std::vector<PassResource> &GetAccessedResources() const;

        const std::string &GetName() const override;

        virtual Result Record(const std::shared_ptr<CommandBufferVector> &commands) const = 0;

    protected:
        template<typename Resource>
            requires std::derived_from<Resource, IVulkanResource>
        std::shared_ptr<Resource> GetResource(const std::string &name) const {
            for (const PassResource &resource : m_resources) {
                if (resource.Resource->GetName() != name) continue;

                std::shared_ptr<Resource> typedResource = resource.Resource;
                if (typedResource == nullptr) {
                    m_logger->Error(
                        "Render pass {} attempted to get resource of type {} which does not exist (a resource by that name of a different type exists)",
                        m_name,
                        typeid(Resource).name()
                    );
                }

                return typedResource;
            }

            m_logger->Error("Render pass {} attempted to get resource of type {} which does not exist", m_name, typeid(Resource).name());
            return nullptr;
        }

    private:
        std::shared_ptr<Logger> m_logger;
        std::vector<PassResource> m_resources;
        std::string m_name;
    };
} // Petal
