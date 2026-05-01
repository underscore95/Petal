#include "VulkanShader.h"

#include "IntermediateShaderResource.h"
#include "Rendering/Internal/RenderingDevice.h"
#include "Rendering/Memory/GPUBuffer.h"
#include "FormatContainers.h"

struct SetInfo {
    glm::u32 NumDescriptors = 0;
    std::vector<Petal::ShaderResource> Resources;
};

namespace Petal {
    VulkanShader::VulkanShader(
        Renderer &renderer,
        const IntermediateShaderResource &shader,
        std::shared_ptr<Logger> logger,
        Result &resultOut
    ) : m_renderer(renderer),
        m_logger(logger) {
        for (const ShaderResource &resource : shader.Resources) {
            m_resources[resource.Name] = resource;
        }

        resultOut = CreateShaderModule(shader);
        if (resultOut != Result::SUCCESS)return;

        resultOut = CreateDescriptors(shader);
        if (resultOut != Result::SUCCESS)return;

        logger->Verbose("Initialized Vulkan shader!");
    }

    VulkanShader::~VulkanShader() {
        if (m_handle) vkDestroyShaderModule(m_renderer.GetDevice()->GetDevice(), m_handle, nullptr);

        m_descriptorSets.clear(); // Destroying the pool destroys these

        for (VkDescriptorSetLayout setLayout : m_descriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(m_renderer.GetDevice()->GetDevice(), setLayout, nullptr);
        }
        m_descriptorSetLayouts.clear();

        if (m_descriptorPool) vkDestroyDescriptorPool(m_renderer.GetDevice()->GetDevice(), m_descriptorPool, nullptr);
    }

    VkShaderModule VulkanShader::GetHandle() const {
        return m_handle;
    }

    Result VulkanShader::CreateDescriptors(
        const IntermediateShaderResource &shader
    ) {
        // Count descriptors required
        std::unordered_map<glm::u32, SetInfo> descriptorSets; // set index -> SetInfo
        std::unordered_map<VkDescriptorType, glm::u32> descriptorsRequired;
        for (const ShaderResource &resource : shader.Resources) {
            descriptorsRequired[ResourceTypes::GetData(resource.Type).VulkanDescriptorType]++;
            descriptorSets[resource.BindingSet].NumDescriptors++;
            descriptorSets[resource.BindingSet].Resources.push_back(resource);
        }

        // Validation
        for (const ShaderResource &resource : shader.Resources) {
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
            m_renderer.GetDevice()->GetDevice(),
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
                    .descriptorCount = 1, // todo: support descriptor indexing
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
                m_renderer.GetDevice()->GetDevice(),
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
        res = vkAllocateDescriptorSets(m_renderer.GetDevice()->GetDevice(), &allocateInfo, m_descriptorSets.data());
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::PETAL_SHADER_DESCRIPTOR_ERROR, m_logger, "Failed to allocate descriptor sets: {}", res);

        m_logger->Verbose("Allocated {} descriptor sets with {} total bindings and {} total descriptors.", m_descriptorSets.size(), numBindings, numDescriptors);

        return Result::SUCCESS;
    }

    Result VulkanShader::BindBuffer(const std::string &name, const GPUBuffer &buffer) {
        auto it = m_resources.find(name);
        PETAL_CHECK_COND(it == m_resources.end(), Result::PETAL_SHADER_RESOURCE_NOT_FOUND, m_logger, "Failed to find buffer {}. Note resource names are case sensitive.", name);

        const ShaderResource &shaderResource = it->second;
        PETAL_CHECK_COND(
            !ResourceTypes::GetData(shaderResource.Type).IsBuffer,
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
            m_renderer.GetDevice()->GetDevice(),
            static_cast<glm::u32>(writes.size()),
            writes.data(),
            0,
            nullptr
        );

        return Result::SUCCESS;
    }

    Result VulkanShader::CreateShaderModule(
        const IntermediateShaderResource &shader
    ) {
        VkShaderModuleCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = shader.SPIRV->getBufferSize(), // size in bytes even though the ptr is u32
            .pCode = static_cast<const glm::u32 *>(shader.SPIRV->getBufferPointer())
        };

        VkResult result = vkCreateShaderModule(
            m_renderer.GetDevice()->GetDevice(),
            &info,
            nullptr,
            &m_handle
        );
        PETAL_CHECK_COND(
            result !=VK_SUCCESS,
            Result::VULKAN_SHADER_CREATION_FAILED,
            m_logger,
            "Failed to create shader module: {}", result
        );
        return Result::SUCCESS;
    }
} // Petal
