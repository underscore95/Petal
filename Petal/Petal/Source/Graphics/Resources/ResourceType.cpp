#include "ResourceType.h"

namespace Petal {
    const std::array<ResourceTypes::EnumData, static_cast<size_t>(ResourceType::COUNT)> ResourceTypes::Data = {
        EnumData{"STORAGE_BUFFER", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Category::BUFFER},
        EnumData{"CONSTANT_BUFFER", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Category::BUFFER},
        EnumData{"COMBINED_IMAGE_SAMPLER", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Category::TEXTURE}
    };

    const ResourceTypes::EnumData &ResourceTypes::GetData(ResourceType resourceType) {
        return Data[static_cast<size_t>(resourceType)];
    }
}
