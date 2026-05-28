#include "BufferType.h"

namespace Petal {
    const std::array<BufferTypes::EnumData, static_cast<size_t>(BufferType::COUNT)> BufferTypes::Data = {
        EnumData{"STORAGE_BUFFER", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT},
        EnumData{"VERTEX_BUFFER", VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
        EnumData{"INDEX_BUFFER", VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
    };

    VkBufferUsageFlags BufferTypes::CombineVulkanFlags(const std::vector<BufferType>& bufferTypes) {
        VkBufferUsageFlags flags = 0;
        for (BufferType type : bufferTypes) {
            flags |= GetData(type).VulkanUsage;
        }
        return flags;
    }

    const BufferTypes::EnumData& BufferTypes::GetData(BufferType bufferType) {
        return Data[static_cast<size_t>(bufferType)];
    }
}