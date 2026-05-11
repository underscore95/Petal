#pragma once

#include "Common.h"

namespace Petal {
    class IBuffer {
    public:
        virtual const std::string &GetName() const =0;

        virtual VkDescriptorBufferInfo GetDescriptorInfo() const = 0;
    };
} // Petal
