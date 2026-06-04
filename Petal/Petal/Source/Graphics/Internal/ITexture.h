#pragma once

#include "Common.h"
#include "Graphics/Memory/IVulkanResource.h"

namespace Petal {
    class ITexture : public IVulkanResource {
    public:
        virtual VkImageView GetImageView() const = 0;

        virtual VkImage GetImage() const = 0;

        virtual VkImageSubresourceRange GetRange() const = 0;
    };
} // Petal
