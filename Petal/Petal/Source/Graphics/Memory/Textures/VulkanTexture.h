#pragma once

#include "TextureCreateInfo.h"
#include "Graphics/Internal/ITexture.h"
#include "Graphics/Internal/VulkanAllocator.h"

namespace Petal {
    class GraphicsContext;

    // Represents a texture on the GPU
    class VulkanTexture : public ITexture {
    public:
        VulkanTexture(
            GraphicsContext &context,
            const std::shared_ptr<Logger> &logger,
            const std::string &name,
            const TextureCreateInfo &textureCreateInfo,
            Result &resultOut
        );

        ~VulkanTexture() override;

    public:
        VkImage GetHandle() const;

        // Size in bytes
        glm::u32 GetSize() const;

        VkImageAspectFlags GetAspectMask() const;

        glm::uvec3 GetDimensions() const;

        VkDescriptorImageInfo GetDescriptorInfo() const;

        VkImageView GetImageView() const override;

        const std::string &GetName() const override;

        VkImageSubresourceRange GetRange() const override;

        bool IsSwapchainImage() const override;

        VkFormat GetFormat() const override;

    private:
        VkImage GetImage() const override { return GetHandle(); }

    private:
        Result CreateTexture();

        Result CreateSampler();

    private:
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        std::string m_name;
        VkImage m_handle = VK_NULL_HANDLE;
        VkImageView m_view = VK_NULL_HANDLE;
        VmaAllocation m_allocation;
        glm::u32 m_size;
        VkImageAspectFlags m_aspectMask;
        const TextureCreateInfo m_textureCreateInfo;
        VkSampler m_sampler = VK_NULL_HANDLE;
    };
} // Petal
