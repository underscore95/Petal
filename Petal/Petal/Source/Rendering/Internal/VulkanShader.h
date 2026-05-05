#pragma once

#include "Common.h"
#include <vulkan/vulkan_core.h>
#include "Rendering/Shaders/ShaderType.h"
#include "Rendering/Renderer.h"
#include "Rendering/Resources/ResourceType.h"

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
            Renderer &renderer,
            const IntermediateShaderResource &shader,
            std::shared_ptr<Logger> logger,
            Result &resultOut
        );

        ~VulkanShader();

    public:
        // Bind a resource
        Result BindBuffer(const std::string &name, const GPUBuffer &buffer);

        // Must be called once for each command buffer before this shader is used
        void BindResources(VkCommandBuffer commandBuffer) const;

        const std::vector<Stage> &GetShaderStages() const;

        const std::vector<VkDescriptorSetLayout> &GetDescriptorSetLayouts() const;

        const VulkanGraphicsPipeline &GetPipeline() const;

    private:
        Result CreateShaderModule(
            const IntermediateShaderResource &shader
        );

        Result CreateDescriptors(
            const IntermediateShaderResource &shader
        );

    private:
        Renderer &m_renderer;
        std::shared_ptr<Logger> m_logger;
        VkDescriptorPool m_descriptorPool;
        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
        std::vector<VkDescriptorSet> m_descriptorSets;
        // resource name -> resource
        std::unordered_map<std::string, ShaderResource> m_resources;
        std::vector<Stage> m_shaderStages;
        std::unique_ptr<VulkanGraphicsPipeline> m_pipeline;
    };
} // Petal
