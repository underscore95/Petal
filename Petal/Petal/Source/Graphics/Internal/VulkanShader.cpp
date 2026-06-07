#include "VulkanShader.h"

#include "Graphics/Shaders/IntermediateShaderResource.h"
#include "Graphics/Internal/RenderingDevice.h"
#include "../Memory/Buffers/GPUBuffer.h"
#include "FormatContainers.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanQueue.h"
#include "VulkanSwapchain.h"
#include "../../../Assets/Shaders/Common.h"
#include "CommandBuffers/CommandBuffer.h"
#include "../Memory/Buffers/IBuffer.h"

namespace Petal {
    struct SetInfo {
        glm::u32 NumDescriptors = 0;
        std::vector<Petal::ShaderResource> Resources;
        // BindingDescriptorCounts[bindingIndex] = number of descriptors
        std::vector<glm::u32> BindingDescriptorCounts;
    };

    VulkanShader::VulkanShader(
        GraphicsContext &renderer,
        const IntermediateShaderResource &shader,
        std::shared_ptr<Logger> logger,
        const VulkanGraphicsPipeline::PipelineSettings &pipelineSettings,
        Result &resultOut
    ) : m_renderer(renderer),
        m_logger(logger),
        m_intermediateShader(shader) {
        for (const ShaderResource &resource : shader.Resources) {
            m_resources[resource.Name] = resource;
        }

        resultOut = CreateShaderModule();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateDescriptors();
        if (resultOut != Result::SUCCESS) return;

        m_pipeline = std::make_unique<VulkanGraphicsPipeline>(
            m_renderer,
            *this,
            m_logger,
            pipelineSettings,
            resultOut
        );
        if (resultOut != Result::SUCCESS) return;

        logger->Verbose("Initialized Vulkan shader!");
    }

    VulkanShader::~VulkanShader() {
        m_pipeline.reset();

        for (const Stage &stage : m_shaderStages) {
            if (stage.ShaderModule == VK_NULL_HANDLE) continue;
            vkDestroyShaderModule(m_renderer.GetDevice()->GetHandle(), stage.ShaderModule, nullptr);
        }
        m_shaderStages.clear();

        m_descriptorSets.clear(); // Destroying the pool destroys these

        for (VkDescriptorSetLayout setLayout : m_descriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(m_renderer.GetDevice()->GetHandle(), setLayout, nullptr);
        }
        m_descriptorSetLayouts.clear();

        if (m_descriptorPool) vkDestroyDescriptorPool(m_renderer.GetDevice()->GetHandle(), m_descriptorPool, nullptr);
    }

    Result VulkanShader::CreateDescriptors() {
        // Count descriptors required
        std::unordered_map<glm::u32, SetInfo> descriptorSets; // set index -> SetInfo
        std::unordered_map<VkDescriptorType, glm::u32> descriptorsRequired;
        for (const ShaderResource &resource : m_intermediateShader.Resources) {
            glm::u32 descriptorCount = 1;
            if (resource.IsArray) {
                descriptorCount = resource.IsStaticArray() ? resource.StaticArraySize : GetMaxDescriptors(resource.BindingSet, resource.BindingIndex);
            }
            descriptorsRequired[ResourceTypes::GetData(resource.Type).VulkanDescriptorType] += descriptorCount;
            descriptorSets[resource.BindingSet].NumDescriptors += descriptorCount;
            descriptorSets[resource.BindingSet].Resources.push_back(resource);
            descriptorSets[resource.BindingSet].BindingDescriptorCounts.push_back(descriptorCount);
        }

        // Validation
        for (const ShaderResource &resource : m_intermediateShader.Resources) {
            PETAL_CHECK_COND(
                descriptorSets[resource.BindingSet].NumDescriptors <= resource.BindingIndex,
                Result::PETAL_SHADER_DESCRIPTOR_ERROR,
                m_logger,
                "descriptorSets invalid: not enough bindings for set {}. Resource binding: {}, num bindings: {}",
                resource.BindingSet,
                resource.BindingIndex,
                descriptorSets[resource.BindingSet].NumDescriptors
            );
        }

        // Create descriptor pool
        std::vector<VkDescriptorPoolSize> poolSizes;
        poolSizes.reserve(descriptorsRequired.size());
        for (const std::pair<const VkDescriptorType, glm::u32> descriptorCount : descriptorsRequired) {
            poolSizes.push_back({.type = descriptorCount.first, .descriptorCount = descriptorCount.second});
        }

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = static_cast<glm::u32>(descriptorSets.size()),
            .poolSizeCount = static_cast<glm::u32>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };
        VkResult res = vkCreateDescriptorPool(
            m_renderer.GetDevice()->GetHandle(),
            &poolCreateInfo,
            nullptr,
            &m_descriptorPool
        );
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_SHADER_DESCRIPTOR_ERROR, m_logger, "Failed to create descriptor pool: {}", res);

        // Create descriptor set layouts
        glm::u32 numBindings = 0;
        glm::u32 numDescriptors = 0;
        for (glm::u32 setIndex = 0; setIndex < descriptorSets.size(); setIndex++) {
            PETAL_CHECK_COND(
                !descriptorSets.contains(setIndex),
                Result::PETAL_SHADER_DESCRIPTOR_ERROR,
                m_logger,
                "Descriptor sets were not sequential."
            );

            std::vector<VkDescriptorSetLayoutBinding> bindings;
            for (glm::u32 bindingIndex = 0; bindingIndex < descriptorSets[setIndex].Resources.size(); bindingIndex++) {
                const ShaderResource &resource = descriptorSets[setIndex].Resources[bindingIndex];

                bindings.push_back(VkDescriptorSetLayoutBinding{
                    .binding = bindingIndex,
                    .descriptorType = ResourceTypes::GetData(resource.Type).VulkanDescriptorType,
                    .descriptorCount = descriptorSets[setIndex].BindingDescriptorCounts[bindingIndex],
                    .stageFlags = ShaderTypes::CombineVulkanFlags(resource.Stages),
                    .pImmutableSamplers = nullptr
                });
                numDescriptors += bindings.back().descriptorCount;
            }

            VkDescriptorSetLayoutCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .bindingCount = static_cast<glm::u32>(bindings.size()),
                .pBindings = bindings.data()
            };
            numBindings += createInfo.bindingCount;

            VkDescriptorSetLayout setLayout;
            res = vkCreateDescriptorSetLayout(
                m_renderer.GetDevice()->GetHandle(),
                &createInfo,
                nullptr,
                &setLayout
            );
            PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_SHADER_DESCRIPTOR_ERROR, m_logger, "Failed to create descriptor set layout: {}", res);

            m_descriptorSetLayouts.push_back(setLayout);
        }

        // Allocate descriptor sets
        VkDescriptorSetAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = static_cast<glm::u32>(m_descriptorSetLayouts.size()),
            .pSetLayouts = m_descriptorSetLayouts.data()
        };

        m_descriptorSets.resize(allocateInfo.descriptorSetCount);
        res = vkAllocateDescriptorSets(m_renderer.GetDevice()->GetHandle(), &allocateInfo, m_descriptorSets.data());
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_SHADER_DESCRIPTOR_ERROR, m_logger, "Failed to allocate descriptor sets: {}", res);

        m_logger->Verbose("Allocated {} descriptor sets with {} total bindings and {} total descriptors.", m_descriptorSets.size(), numBindings, numDescriptors);

        return Result::SUCCESS;
    }

    glm::u32 VulkanShader::GetMaxDescriptors(glm::u32 set, glm::u32 binding) const {
        return 128; // todo configurable
    }

    Result VulkanShader::BindBuffer(const std::string &name, const IBuffer &buffer) const {
        const auto it = m_resources.find(name);
        PETAL_CHECK_COND(it == m_resources.end(), Result::PETAL_SHADER_RESOURCE_NOT_FOUND, m_logger, "Failed to find buffer {}. Note resource names are case sensitive.", name);

        const ShaderResource &shaderResource = it->second;
        PETAL_CHECK_COND(
            ResourceTypes::GetData(shaderResource.Type).ResourceCategory != ResourceTypes::Category::BUFFER,
            Result::PETAL_SHADER_RESOURCE_NOT_FOUND,
            m_logger,
            "Failed to find buffer {} (warning: found a {} with the same name)", name, shaderResource.Type
        );

        VkDescriptorBufferInfo bufferInfo = buffer.GetDescriptorInfo();

        VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_descriptorSets[shaderResource.BindingSet],
            .dstBinding = shaderResource.BindingIndex,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = ResourceTypes::GetData(shaderResource.Type).VulkanDescriptorType,
            .pImageInfo = nullptr,
            .pBufferInfo = &bufferInfo,
            .pTexelBufferView = nullptr
        };

        std::vector writes = {write};

        vkUpdateDescriptorSets(
            m_renderer.GetDevice()->GetHandle(),
            static_cast<glm::u32>(writes.size()),
            writes.data(),
            0,
            nullptr
        );

        m_logger->Verbose("Bound buffer {} to set {} index {} (resource name: {})", buffer.GetName(), shaderResource.BindingSet, shaderResource.BindingIndex, shaderResource.Name);

        return Result::SUCCESS;
    }

    const IntermediateShaderResource &VulkanShader::GetIntermediateShader() const {
        return m_intermediateShader;
    }

    Result VulkanShader::BindTexturesImpl(const std::string &name, const std::vector<VkDescriptorImageInfo> &textures) const {
        const auto it = m_resources.find(name);
        PETAL_CHECK_COND(it == m_resources.end(), Result::PETAL_SHADER_RESOURCE_NOT_FOUND, m_logger, "Failed to find texture {}. Note resource names are case sensitive.", name);

        const ShaderResource &shaderResource = it->second;
        PETAL_CHECK_COND(
            ResourceTypes::GetData(shaderResource.Type).ResourceCategory != ResourceTypes::Category::TEXTURE,
            Result::PETAL_SHADER_RESOURCE_NOT_FOUND,
            m_logger,
            "Failed to find texture {} (warning: found a {} with the same name)", name, shaderResource.Type
        );

        VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_descriptorSets[shaderResource.BindingSet],
            .dstBinding = shaderResource.BindingIndex,
            .dstArrayElement = 0,
            .descriptorCount = static_cast<glm::u32>(textures.size()),
            .descriptorType = ResourceTypes::GetData(shaderResource.Type).VulkanDescriptorType,
            .pImageInfo = textures.data(),
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };

        std::vector writes = {write};

        vkUpdateDescriptorSets(
            m_renderer.GetDevice()->GetHandle(),
            static_cast<glm::u32>(writes.size()),
            writes.data(),
            0,
            nullptr
        );

        m_logger->Verbose("Bound {} textures to set {} index {} (resource name: {})", textures.size(), shaderResource.BindingSet, shaderResource.BindingIndex,
                          shaderResource.Name);

        return Result::SUCCESS;
    }

    void VulkanShader::BindResources(VkCommandBuffer commandBuffer) const {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipeline->GetLayout(),
            0,
            static_cast<glm::u32>(m_descriptorSets.size()),
            m_descriptorSets.data(),
            0,
            nullptr
        );
    }

    void VulkanShader::BindResources(const CommandBufferVector &commandBuffer) const {
        for (glm::u32 i = 0; i < commandBuffer.Size(); i++) {
            BindResources(commandBuffer.GetHandle(i));
        }
    }

    const std::vector<VulkanShader::Stage> &VulkanShader::GetShaderStages() const {
        return m_shaderStages;
    }

    const std::vector<VkDescriptorSetLayout> &VulkanShader::GetDescriptorSetLayouts() const {
        return m_descriptorSetLayouts;
    }

    const VulkanGraphicsPipeline &VulkanShader::GetPipeline() const {
        return *m_pipeline;
    }

    Result VulkanShader::CreateShaderModule() {
        for (const std::pair<const ShaderType, IntermediateShaderResource::ShaderStage> &pair : m_intermediateShader.ShaderTypes) {
            ShaderType type = pair.first;
            const IntermediateShaderResource::ShaderStage &stage = pair.second;

            VkShaderModuleCreateInfo info = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .codeSize = stage.SPIRV->getBufferSize(), // size in bytes even though the ptr is u32
                .pCode = static_cast<const glm::u32 *>(stage.SPIRV->getBufferPointer())
            };

            Stage vulkanStage = {
                .Type = type,
                .EntryPointFunctionName = stage.EntryFunctionName,
                .ShaderModule = VK_NULL_HANDLE
            };

            VkResult result = vkCreateShaderModule(
                m_renderer.GetDevice()->GetHandle(),
                &info,
                nullptr,
                &vulkanStage.ShaderModule
            );
            PETAL_CHECK_COND(
                result != VK_SUCCESS,
                Result::VULKAN_SHADER_CREATION_FAILED,
                m_logger,
                "Failed to create shader module: {}", result
            );

            m_shaderStages.push_back(vulkanStage);
        }

        return Result::SUCCESS;
    }
} // Petal
