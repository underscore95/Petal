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
        Result BindBuffer(const std::string &name, const IBuffer &buffer) const;

        // T must be iterable (vector, StableVector, etc)
        template<typename Container>
        Result BindTextures(const std::string &name, const Container &textures) const {
            std::vector<VkDescriptorImageInfo> imageInfos(textures.size());
            size_t i = 0;
            for (const auto &texture : textures) {
                static_assert(typeid(texture) == typeid(std::shared_ptr<VulkanTexture>));
                imageInfos[i] = texture->GetDescriptorInfo();
                i++;
            }

            return BindTexturesImpl(name, imageInfos.data(), imageInfos.size());
        }

        Result BindTexture(const std::string &name, const VulkanTexture &texture) const;

        // Must be called once for each command buffer before this shader is used
        void CmdBindResources(
            const VulkanGraphicsPipeline &pipeline,
            VkCommandBuffer commandBuffer
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
        Result BindTexturesImpl(const std::string &name, const VkDescriptorImageInfo *imageInfos, glm::u32 numImageInfos) const;

        Result CreateShaderModule();

        Result CreateDescriptors();

        // Maximum size of a dynamic array, since we need to allocate space for descriptors during initialization
        glm::u32 GetMaxDescriptors(glm::u32 set, glm::u32 binding) const;

    private:
        GraphicsContext &m_renderer;
        std::shared_ptr<Logger> m_logger;
        IntermediateShaderResource m_intermediateShader;
        VkDescriptorPool m_descriptorPool;
        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
        std::vector<VkDescriptorSet> m_descriptorSets;
        // resource name -> resource
        std::unordered_map<std::string, ShaderResource> m_resources;
        std::vector<Stage> m_shaderStages;
    };
} // Petal
