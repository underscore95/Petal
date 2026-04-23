#include "Version.h"

#ifdef PETAL_VULKAN
#include <vulkan/vulkan_core.h>
#endif

#ifdef PETAL_VULKAN
glm::u32 Petal::Version::ToVulkanVersion() const {
    return VK_MAKE_API_VERSION(Variant, Major, Minor, Patch);
}
#endif

Petal::Version Petal::Version::FromVulkanVersion(uint32_t version) {
    Version v;
    v.Variant = VK_API_VERSION_VARIANT(version);
    v.Major = VK_API_VERSION_MAJOR(version);
    v.Minor = VK_API_VERSION_MINOR(version);
    v.Patch = VK_API_VERSION_PATCH(version);
    return v;
}

bool Petal::Version::operator==(const Version &other) const {
    return Variant == other.Variant && Major == other.Major && Minor == other.Minor & Patch == other.Patch;
}

bool Petal::Version::operator<(const Version &other) const {
    if (Variant != other.Variant) return Variant < other.Variant;
    if (Major != other.Major) return Major < other.Major;
    if (Minor != other.Minor) return Minor < other.Minor;
    return Patch < other.Patch;
}

bool Petal::Version::operator>(const Version &other) const {
    return other < *this;
}

bool Petal::Version::operator<=(const Version &other) const {
    return !(*this > other);
}

bool Petal::Version::operator>=(const Version &other) const {
    return !(*this < other);
}
