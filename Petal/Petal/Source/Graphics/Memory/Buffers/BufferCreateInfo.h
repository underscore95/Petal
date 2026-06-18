#pragma once

#include "BufferType.h"

namespace Petal {
    struct BufferCreateInfo {
        BufferType BufferType = BufferType::STORAGE_BUFFER;
        bool DeviceLocal = true;
        bool HostVisible = false;
        bool IsTransferDest = true;
        bool IsTransferSource = false;
    };
} // Petal
