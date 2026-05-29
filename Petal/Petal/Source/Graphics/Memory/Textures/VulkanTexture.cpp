#include "VulkanTexture.h"

#include "TextureCreateInfo.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Internal/VulkanQueue.h"

namespace Petal {
    VulkanTexture::VulkanTexture(
        GraphicsContext &context,
        const std::shared_ptr<Logger> &logger,
        const std::string &name,
        const TextureCreateInfo &textureCreateInfo,
        Result &resultOut
    ) : m_context(context),
        m_logger(logger),
        m_name(name),
        m_aspectMask(textureCreateInfo.AspectFlags),
        m_textureCreateInfo(textureCreateInfo) {
        resultOut = CreateTexture();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateSampler();
        if (resultOut != Result::SUCCESS) return;

        m_context.SetObjectDebugName(reinterpret_cast<glm::u64>(m_sampler), VK_OBJECT_TYPE_SAMPLER, std::format("{} Sampler", m_name));
        m_context.SetObjectDebugName(reinterpret_cast<glm::u64>(m_handle), VK_OBJECT_TYPE_IMAGE, std::format("{} Sampler", m_name));
        m_context.SetObjectDebugName(reinterpret_cast<glm::u64>(m_view), VK_OBJECT_TYPE_IMAGE_VIEW, std::format("{} Sampler", m_name));
    }

    VulkanTexture::~VulkanTexture() {
        if (m_sampler) vkDestroySampler(m_context.GetDevice()->GetHandle(), m_sampler, nullptr);
        if (m_view != VK_NULL_HANDLE) vkDestroyImageView(m_context.GetDevice()->GetHandle(), m_view, nullptr);
        if (m_handle != VK_NULL_HANDLE) vmaDestroyImage(m_context.GetAllocator()->GetHandle(), m_handle, m_allocation);
    }

    VkImage VulkanTexture::GetHandle() const {
        return m_handle;
    }

    glm::u32 VulkanTexture::GetSize() const {
        return m_size;
    }

    std::string VulkanTexture::GetName() const {
        return m_name;
    }

    VkImageAspectFlags VulkanTexture::GetAspectMask() const {
        return m_aspectMask;
    }

    glm::uvec3 VulkanTexture::GetDimensions() const {
        return m_textureCreateInfo.Size;
    }

    VkDescriptorImageInfo VulkanTexture::GetDescriptorInfo() const {
        return {
            .sampler = m_sampler,
            .imageView = m_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
    }

    VkImageView VulkanTexture::GetView() const {
        return m_view;
    }

    Result VulkanTexture::CreateTexture() {
        Result result = m_textureCreateInfo.Validate(m_logger);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        std::array<glm::u32, 1> queueFamilies = {m_context.GetDevice()->GetGraphicsQueueFamily().GetQueueFamilyIndex()};

        VkImageCreateInfo imageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = m_textureCreateInfo.ImageType,
            .format = m_textureCreateInfo.Format,
            .extent = {m_textureCreateInfo.Size.x, m_textureCreateInfo.Size.y, m_textureCreateInfo.Size.z},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = m_textureCreateInfo.Usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = queueFamilies.size(),
            .pQueueFamilyIndices = queueFamilies.data(),
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VmaAllocationCreateInfo allocInfo = {
            .flags = 0,
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
            .requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0
        };

        VkResult res = vmaCreateImage(
            m_context.GetAllocator()->GetHandle(),
            &imageInfo,
            &allocInfo,
            &m_handle,
            &m_allocation,
            nullptr
        );
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VMA_TEXTURE_CREATION_FAILED, m_logger, "Failed to create texture {}", m_name);

        VkImageViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = m_handle,
            .viewType = m_textureCreateInfo.ViewType,
            .format = m_textureCreateInfo.Format,
            .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {
                .aspectMask = m_aspectMask,
                .baseMipLevel = 0,
                .levelCount = imageInfo.mipLevels,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        m_size = m_textureCreateInfo.Size.x * m_textureCreateInfo.Size.y * m_textureCreateInfo.Size.z * VkFormatValueSize(m_textureCreateInfo.Format);

        res = vkCreateImageView(m_context.GetDevice()->GetHandle(), &viewInfo, nullptr, &m_view);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VMA_TEXTURE_CREATION_FAILED, m_logger, "Failed to create view for texture {}", m_name);

        return Result::SUCCESS;
    }

    Result VulkanTexture::CreateSampler() {
        VkSamplerCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR, // todo configurable filter
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, // todo configurable address mode
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
            .mipLodBias = 0,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 0,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0,
            .maxLod = VK_LOD_CLAMP_NONE,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
        };

        VkResult result = vkCreateSampler(m_context.GetDevice()->GetHandle(), &info, nullptr, &m_sampler);
        PETAL_CHECK_COND(result != VK_SUCCESS, Result::VMA_TEXTURE_CREATION_FAILED, m_logger, "Failed to create sampler");

        return Result::SUCCESS;
    }
} // Petal
