#pragma once

#include "pch.h"
#include "FormatString.h"

namespace Petal {
    struct Version {
        glm::u32 Variant;
        glm::u32 Major;
        glm::u32 Minor;
        glm::u32 Patch;

#ifdef PETAL_VULKAN
        glm::u32 ToVulkanVersion() const;

        static Version FromVulkanVersion(uint32_t version);
#endif

        bool operator==(const Version &other) const;

        bool operator>(const Version &other) const;

        bool operator<=(const Version &other) const;

        bool operator>=(const Version &other) const;

        bool operator<(const Version &other) const;
    };
} // Petal

PETAL_MAKE_FORMATTABLE(
    Petal::Version, version,
    std::format("v{}.{}.{}.{}", version.Variant,version.Major,version.Minor,version.Patch)
);
