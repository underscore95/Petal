#pragma once

#include <complex.h>

#include "Common.h"
#include <vulkan/vulkan_core.h>

#include "VulkanGraphicsPipeline.h"
#include "Graphics/Shaders/ShaderType.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/VertexType.h"
#include "Graphics/Memory/Textures/VulkanTexture.h"
#include "Graphics/Resources/ResourceType.h"
#include "Graphics/Shaders/IntermediateShaderResource.h"
#include "Rendering/Deferred/DeferredRenderer.h"

namespace Petal {
    struct VertexType;
    class IBuffer;
    struct ShaderResource;
    class GPUBuffer;

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

        // If std::shared_ptr<IBuffer> is passed into the variant, bind it to all swapchain textures
        // If std::vector<std::shared_ptr<IBuffer>> is passed into the variant,
        // its size must be equal to the number of swapchain textures and each buffer will be bound to the respective swapchain texture
        Result BindBuffer(
            const std::string &name,
            const std::variant<std::shared_ptr<IBuffer>, std::vector<std::shared_ptr<IBuffer> > > &bufferToBind
        ) const;

        // If std::shared_ptr<VulkanTexture> is passed into the variant, bind the texture to all swapchain textures
        // If std::vector<std::shared_ptr<VulkanTexture>> is passed into the variant, bind each texture to
        // its respective swapchain texture. The size of the vector must be equal to the number of swapchain textures.
        Result BindTexture(
            const std::string &name,
            const std::variant<std::shared_ptr<VulkanTexture>, std::vector<std::shared_ptr<VulkanTexture> > > &texturesToBind
        ) const;

        // If std::vector<std::shared_ptr<VulkanTexture>> is passed into the variant, bind the textures to all swapchain textures
        // If std::vector<std::vector<std::shared_ptr<VulkanTexture>>> is passed into the variant,
        // its size must be equal to the number of swapchain textures and each std::vector<std::shared_ptr<VulkanTexture>>
        // will be bound to the respective swapchain texture.
        Result BindTextures(
            const std::string &name,
            const std::variant<std::vector<std::shared_ptr<VulkanTexture> >, std::vector<std::vector<std::shared_ptr<VulkanTexture> > > > &texturesToBind
        ) const;

        // Must be called once for each command buffer before this shader is used
        void CmdBindResources(
            const VulkanGraphicsPipeline &pipeline,
            VkCommandBuffer commandBuffer,
            glm::u32 swapchainIndex
        ) const;

        void CmdBindResources(
            const VulkanGraphicsPipeline &pipeline,
            const CommandBufferVector &commandBuffer
        ) const;

        const std::vector<Stage> &GetShaderStages() const;

        const std::vector<VkDescriptorSetLayout> &GetDescriptorSetLayouts() const;

        const IntermediateShaderResource &GetIntermediateShader() const;

        // Create a pipeline for this shader
        AllocatedOptional<VulkanGraphicsPipeline> CreatePipeline(const VulkanGraphicsPipeline::PipelineSettings &pipelineSettings) const;

    private:
        Result CreateShaderModule();

        Result CreateDescriptors();

        // Maximum size of a dynamic array, since we need to allocate space for descriptors during initialization
        glm::u32 GetMaxDescriptors(glm::u32 set, glm::u32 binding) const;

    private:
        GraphicsContext &m_renderer;
        std::shared_ptr<Logger> m_logger;
        IntermediateShaderResource m_intermediateShader;
        glm::u32 m_numSwapchainImages;
        VkDescriptorPool m_descriptorPool;
        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
        std::vector<std::vector<VkDescriptorSet> > m_descriptorSets;
        // resource name -> resource
        std::unordered_map<std::string, ShaderResource> m_resources;
        std::vector<Stage> m_shaderStages;
    };
} // Petal
