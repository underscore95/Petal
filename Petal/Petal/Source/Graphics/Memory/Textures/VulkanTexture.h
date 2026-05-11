#pragma once

#include "TextureCreateInfo.h"
#include "Graphics/Internal/VulkanAllocator.h"

namespace Petal {
    class GraphicsContext;

    // Represents a texture on the GPU
    class VulkanTexture {
    public:
        VulkanTexture(
            GraphicsContext &context,
            const std::shared_ptr<Logger> &logger,
            const std::string &name,
            const TextureCreateInfo &textureCreateInfo,
            Result &resultOut
        );

        ~VulkanTexture();

    public:
        VkImage GetHandle() const;

        // Size in bytes
        glm::u32 GetSize() const;

        std::string GetName() const;

        VkImageAspectFlags GetAspectMask() const;

        glm::uvec3 GetDimensions() const;

        VkDescriptorImageInfo GetDescriptorInfo() const;

    private:
        Result CreateTexture();
        Result CreateSampler();

    private:
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        std::string m_name;
        VkImage m_handle;
        VkImageView m_view;
        VmaAllocation m_allocation;
        glm::u32 m_size;
        VkImageAspectFlags m_aspectMask;
        const TextureCreateInfo m_textureCreateInfo;
        VkSampler m_sampler;
    };
} // Petal
