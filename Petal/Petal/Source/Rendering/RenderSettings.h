#pragma once

#include "Common.h"

namespace Petal {
    struct RenderSettings {
        glm::u32 NumSwapchainImages = 3;
        // Fall back to 2 if the device doesn't support 3
        bool AllowDoubleBufferingFallback = true;
        // List of present modes, the first present mode in the vector that is supported will be used
        std::vector<VkPresentModeKHR> PreferredPresentMode{VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR};

        glm::u32 PushConstantSize = 128;

        // Returns SUCCESS if valid, PETAL_INVALID_RENDER_SETTINGS if invalid.
        Result IsValid(std::shared_ptr<Logger> logger) const;
    };
} // Petal
