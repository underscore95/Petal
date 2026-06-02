#pragma once

#include "Common.h"

namespace Petal {
    class IHasVulkanImage {
    public:
        virtual ~IHasVulkanImage() = default;

    public:
        virtual VkImageView GetImageView() const = 0;

        virtual VkImage GetImage() const = 0;
    };
} // Petal
