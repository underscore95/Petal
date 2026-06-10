#pragma once

#include "Common.h"

namespace Petal {
    class VulkanBuffer;
    class IBuffer;

    struct MultipleBuffers {
        std::vector<std::shared_ptr<IBuffer> > Buffers;
        std::shared_ptr<VulkanBuffer> BackingBuffer;

        const IBuffer &operator[](size_t index) const {
            return *Buffers[index];
        }
    };
} // Petal
