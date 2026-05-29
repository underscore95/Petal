#pragma once

#ifdef PETAL_VULKAN
#include <vulkan/vk_enum_string_helper.h>
#include "Utils/FormatString.h"

PETAL_MAKE_FORMATTABLE(
    VkResult, result,
    std::format("{}", string_VkResult(result))
);

PETAL_MAKE_FORMATTABLE(
    VkFormat, format,
    std::format("{}", string_VkFormat(format))
);

PETAL_MAKE_FORMATTABLE(
    VkImageViewType, imageViewType,
    std::format("{}", string_VkImageViewType(imageViewType))
);

PETAL_MAKE_FORMATTABLE(
    VkSurfaceFormat2KHR, surfaceFormat,
    std::format("[format: {}, color space: {}]", string_VkFormat(surfaceFormat.surfaceFormat.format), string_VkColorSpaceKHR(surfaceFormat.surfaceFormat.colorSpace))
);

namespace Petal {
    inline glm::u32 VkFormatValueSize(VkFormat format) {
        switch (format) {
            case VK_FORMAT_R8_UNORM:
                return 1;

            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R16_SFLOAT:
                return 2;

            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_B8G8R8_UNORM:
                return 3;

            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_D32_SFLOAT:
                return 4;

            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
                return 8;

            case VK_FORMAT_R32G32B32_SFLOAT:
                return 12;

            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return 16;

            default:
                assert(false && "Unhandled VkFormat");
                return 0;
        }
    }
} // Petal
#endif
