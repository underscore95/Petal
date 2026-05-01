#pragma once

#include "Common.h"

namespace Petal {
    enum class ResourceType {
        CONSTANT_BUFFER,
        COUNT
    };

    class ResourceTypes {
        ResourceTypes() = delete;

    public:
        struct EnumData {
            const char* Name;
            VkDescriptorType VulkanDescriptorType;
            bool IsBuffer;
        };

    public:
        static const EnumData& GetData(ResourceType resourceType);

    private:
        static const std::array<EnumData, static_cast<size_t>(ResourceType::COUNT)> Data;
    };
}

PETAL_MAKE_ENUM_FORMATTABLE(Petal::ResourceType);