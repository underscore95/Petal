#pragma once

#include "pch.h"

namespace Petal {
    // https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkSurfaceCapabilitiesKHR.html
    // If max image count is 0, no limit except for memory
    static constexpr glm::u32 VULKAN_SENTINEL_SURFACE_CAPABILITIES_UNLIMITED_SWAPCHAIN_IMAGES = 0;

    static constexpr glm::u64 PETAL_U64_MAX = UINT64_MAX;
} // Petal
