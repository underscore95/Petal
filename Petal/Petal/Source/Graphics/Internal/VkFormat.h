#pragma once

#ifdef PETAL_VULKAN
#include <vulkan/vk_enum_string_helper.h>
#include "Utils/FormatString.h"

PETAL_MAKE_FORMATTABLE(
    VkResult, result,
    std::format("{}", string_VkResult(result))
);

PETAL_MAKE_FORMATTABLE(
    VkSurfaceFormat2KHR, surfaceFormat,
    std::format("[format: {}, color space: {}]", string_VkFormat(surfaceFormat.surfaceFormat.format), string_VkColorSpaceKHR(surfaceFormat.surfaceFormat.colorSpace))
);
#endif
