#pragma once

#include "Common.h"

namespace Petal {
    enum class BufferType {
        STORAGE_BUFFER,
        VERTEX_BUFFER,
        INDEX_BUFFER,
        COUNT
    };

    class BufferTypes {
        BufferTypes() = delete;

    public:
        struct EnumData {
            std::string Name;
            VkBufferUsageFlags VulkanUsage;
        };

    public:
        static VkBufferUsageFlags CombineVulkanFlags(const std::vector<BufferType>& bufferTypes);
        static const EnumData& GetData(BufferType bufferType);

    private:
        static const std::array<EnumData, static_cast<size_t>(BufferType::COUNT)> Data;
    };
}

PETAL_MAKE_ENUM_FORMATTABLE(Petal::BufferType);