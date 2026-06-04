#include "ResourceAccess.h"

namespace Petal {
    const std::array<ResourceAccesses::EnumData, static_cast<size_t>(ResourceAccess::COUNT)> ResourceAccesses::Data = {
        EnumData{"READ", true, false},
        EnumData{"WRITE", false, true},
        EnumData{"READ_WRITE", true, true}
    };

    const ResourceAccesses::EnumData &ResourceAccesses::GetData(ResourceAccess resourceAccess) {
        return Data[static_cast<size_t>(resourceAccess)];
    }
}