#include "IndexType.h"

namespace Petal {
    const std::array<IndexTypes::EnumData, static_cast<size_t>(IndexType::COUNT)> IndexTypes::Data = {
        EnumData{
            "INDICES_16_BIT",
            2,
            typeid(glm::u16),
            VK_INDEX_TYPE_UINT16,
            std::numeric_limits<glm::u16>().max()
        },
        EnumData{
            "INDICES_32_BIT",
            4,
            typeid(glm::u32),
            VK_INDEX_TYPE_UINT32,
            std::numeric_limits<glm::u32>().max()
        }
    };

    const IndexTypes::EnumData &IndexTypes::GetData(IndexType indexType) {
        return Data[static_cast<size_t>(indexType)];
    }

    constexpr IndexType &operator++(IndexType &value) {
        value = static_cast<IndexType>(static_cast<int>(value) + 1);
        return value;
    }

    IndexType IndexTypes::GetBestIndexType(glm::u32 numIndices) {
        for (IndexType indexType = static_cast<IndexType>(0); indexType < IndexType::COUNT; ++indexType) {
            if (GetData(indexType).MaxIndices >= numIndices) return indexType;
        }
        assert("No index type supports this many indices!");
        return IndexType::INDICES_32_BIT;
    }
} // Petal
