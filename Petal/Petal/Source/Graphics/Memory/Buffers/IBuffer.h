#pragma once

#include "Common.h"
#include "Graphics/Memory/IVulkanResource.h"

namespace Petal {
    class IBuffer : public IVulkanResource {
    public:
        virtual VkDescriptorBufferInfo GetDescriptorInfo() const = 0;

        virtual glm::u32 GetOffset() const = 0;

        virtual glm::u32 GetSize() const = 0;

        virtual VkBuffer GetHandle() const = 0;
    };
} // Petal
