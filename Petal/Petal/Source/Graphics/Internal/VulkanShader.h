#pragma once

#include "Common.h"
#include <vulkan/vulkan_core.h>
#include "Graphics/Shaders/ShaderType.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/VertexType.h"
#include "Graphics/Memory/Textures/VulkanTexture.h"
#include "Graphics/Resources/ResourceType.h"

namespace Petal {
    struct VertexType;
}

namespace Petal {
    class IBuffer;
}

namespace Petal {
    class VulkanGraphicsPipeline;
    struct ShaderResource;
    class GPUBuffer;
    struct IntermediateShaderResource;

    class VulkanShader {
    private:
        struct ResourceLocation {
            glm::u32 Binding;
            glm::u32 Set;
        };

    public:
        struct Stage {
            ShaderType Type;
            std::string EntryPointFunctionName;
            VkShaderModule ShaderModule;
        };

    public:
        VulkanShader(
            GraphicsContext &renderer,
            const IntermediateShaderResource &shader,
            std::shared_ptr<Logger> logger,
            Result &resultOut
        );

        ~VulkanShader();

    public:
        // Bind a resource
        Result BindBuffer(const std::string &name, const IBuffer &buffer) const;

        Result BindTextures(const std::string &name, const std::vector<std::shared_ptr<VulkanTexture> > &textures) const;

        // Must be called once for each command buffer before this shader is used
        void BindResources(VkCommandBuffer commandBuffer) const;

        void BindResources(const CommandBufferVector &commandBuffer) const;

        const std::vector<Stage> &GetShaderStages() const;

        const std::vector<VkDescriptorSetLayout> &GetDescriptorSetLayouts() const;

        const VulkanGraphicsPipeline &GetPipeline() const;

        const Optional<VertexType> &GetVertexType() const;

    private:
        Result CreateShaderModule(
            const IntermediateShaderResource &shader
        );

        Result CreateDescriptors(
            const IntermediateShaderResource &shader
        );

        // Maximum size of a dynamic array, since we need to allocate space for descriptors during initialization
        glm::u32 GetMaxDescriptors(glm::u32 set, glm::u32 binding) const;

    private:
        GraphicsContext &m_renderer;
        std::shared_ptr<Logger> m_logger;
        VkDescriptorPool m_descriptorPool;
        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
        std::vector<VkDescriptorSet> m_descriptorSets;
        // resource name -> resource
        std::unordered_map<std::string, ShaderResource> m_resources;
        std::vector<Stage> m_shaderStages;
        std::unique_ptr<VulkanGraphicsPipeline> m_pipeline;
       Optional< VertexType> m_vertexType;
    };
} // Petal
