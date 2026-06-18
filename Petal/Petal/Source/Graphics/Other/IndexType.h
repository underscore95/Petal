#pragma once

#include "Common.h"

namespace Petal {
    enum class IndexType {
        // Petal doesn't support 8 bit indices as they require a relatively recent extension
        // additionally if you're rendering a mesh with <256 vertices, that is going to be very fast anyway unless you render millions of instances
        INDICES_16_BIT,
        INDICES_32_BIT,
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
            glm::u32 MaxIndices;
        };

    public:
        static const EnumData &GetData(IndexType indexType);

        // Return the smallest index type which can store enough indices
        static IndexType GetBestIndexType(glm::u32 numIndices);

    private:
        static const std::array<EnumData, static_cast<size_t>(IndexType::COUNT)> Data;
    };
} // Petal

PETAL_MAKE_ENUM_FORMATTABLE(Petal::IndexType);
