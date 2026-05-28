#pragma once

#include "Common.h"

namespace Petal {
    class IBuffer {
    public:
        virtual const std::string &GetName() const =0;

        virtual VkDescriptorBufferInfo GetDescriptorInfo() const = 0;

        virtual glm::u32 GetOffset() const = 0;

        virtual glm::u32 GetSize() const = 0;

        virtual VkBuffer GetHandle() const = 0;
    };
} // Petal
