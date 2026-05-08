#pragma once

#include "Common.h"

namespace Petal {
    struct QueueFamilyRequirements {
        VkQueueFlags QueueType;
        bool SupportsPresenting;
    };

    struct DeviceRequirements {
        Version APIVersion;
        std::vector<QueueFamilyRequirements> QueueFamilies;

        static constexpr Version DEFAULT_API_VERSION = {
            .Variant = 0,
            .Major = 1,
            .Minor = 3,
            .Patch = 0
        };
        static DeviceRequirements DEFAULT();
    };
} // Petal
