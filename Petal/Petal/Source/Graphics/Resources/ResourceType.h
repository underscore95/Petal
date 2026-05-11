#pragma once

#include "Common.h"

namespace Petal {
    enum class ResourceType {
        // Make sure to fill out EnumData in the .cpp
        STORAGE_BUFFER,
        COMBINED_SAMPLER,
        COUNT,
    };

    class ResourceTypes {
        ResourceTypes() = delete;

    public:
        enum class Category {
            BUFFER, TEXTURE
        };

        struct EnumData {
            const char *Name;
            VkDescriptorType VulkanDescriptorType;
            Category ResourceCategory;

            EnumData(
                const char *name,
                VkDescriptorType vulkanDescriptorType,
                Category resourceCategory
            )
                : Name(name),
                  VulkanDescriptorType(vulkanDescriptorType),
                  ResourceCategory(resourceCategory) {
            }
        };

    public:
        static const EnumData &GetData(ResourceType resourceType);

    private:
        static const std::array<EnumData, static_cast<size_t>(ResourceType::COUNT)> Data;
    };
}

PETAL_MAKE_ENUM_FORMATTABLE(Petal::ResourceType);
