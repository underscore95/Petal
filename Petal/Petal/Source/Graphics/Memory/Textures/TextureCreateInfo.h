#pragma once

#include "Common.h"

namespace Petal {
    struct TextureCreateInfo {
        glm::uvec3 Size;
        VkImageType ImageType = VK_IMAGE_TYPE_2D ;
        VkImageViewType ViewType = VK_IMAGE_VIEW_TYPE_2D;
        VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
        VkImageUsageFlags Usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

        Result Validate(const std::shared_ptr<Logger> &logger) const;
    };
} // Petal
