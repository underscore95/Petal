#include "ResourceType.h"

namespace Petal {
    const std::array<ResourceTypes::EnumData, static_cast<size_t>(ResourceType::COUNT)> ResourceTypes::Data = {
        EnumData{"CONSTANT_BUFFER", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, true}
    };

    const ResourceTypes::EnumData& ResourceTypes::GetData(ResourceType resourceType) {
        return Data[static_cast<size_t>(resourceType)];
    }
}