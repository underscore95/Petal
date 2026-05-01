#pragma once

#include <complex.h>
#include <vector>

#include "Common.h"
#include <vulkan/vulkan_core.h>

#include "ShaderType.h"
#include "Rendering/Renderer.h"
#include "Rendering/Resources/ResourceType.h"

namespace Petal {
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
        VulkanShader(
            Renderer &renderer,
            const IntermediateShaderResource &shader,
            std::shared_ptr<Logger> logger,
            Result &resultOut
        );

        ~VulkanShader();

    public:
        VkShaderModule GetHandle() const;

        // Bind a resource
        Result BindBuffer(const std::string &name, const GPUBuffer &buffer);

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
        VkShaderModule m_handle;
        VkDescriptorPool m_descriptorPool;
        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
        std::vector<VkDescriptorSet> m_descriptorSets;
        std::unordered_map<std::string, ShaderResource> m_resources;
    };
} // Petal


