#include "IndexType.h"

namespace Petal {
    const std::array<IndexTypes::EnumData, static_cast<size_t>(IndexType::COUNT)> IndexTypes::Data = {
        EnumData{"INDICES_32_BIT", 4, typeid(glm::u32), VK_INDEX_TYPE_UINT32},
        EnumData{"INDICES_16_BIT", 2, typeid(glm::u16), VK_INDEX_TYPE_UINT16},
        EnumData{"INDICES_8_BIT", 1, typeid(glm::u8), VK_INDEX_TYPE_UINT8}
    };

    const IndexTypes::EnumData &IndexTypes::GetData(IndexType indexType) {
        return Data[static_cast<size_t>(indexType)];
    }
} // Petal
