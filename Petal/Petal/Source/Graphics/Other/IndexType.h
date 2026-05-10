#pragma once

#include "Common.h"

namespace Petal {
    enum class IndexType {
        INDICES_32_BIT,
        INDICES_16_BIT,
        INDICES_8_BIT,
        COUNT
    };

    class IndexTypes {
        IndexTypes() = delete;

    public:
        struct EnumData {
            const char *Name;
            glm::u32 SizeInBytes;
            const std::type_info &Type;
            VkIndexType VulkanIndexType;
        };

    public:
        static const EnumData &GetData(IndexType indexType);

    private:
        static const std::array<EnumData, static_cast<size_t>(IndexType::COUNT)> Data;
    };
} // Petal

PETAL_MAKE_ENUM_FORMATTABLE(Petal::IndexType);
