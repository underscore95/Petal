#pragma once

#include "Common.h"
#include "BufferType.h"
#include "Rendering/Internal/VulkanAllocator.h"
#include "vulkan/vulkan.h"

namespace Petal {
    struct BufferCreateInfo {
        BufferType BufferType = BufferType::STORAGE_BUFFER;
        bool DeviceLocal = true;
        bool HostVisible = false;
        bool IsTransferDest = true;
        bool IsTransferSource = false;
    };

    class Renderer;

    class VulkanBuffer {
    public:
        explicit VulkanBuffer(
            Renderer &renderer,
            const std::shared_ptr<Logger> &logger,
            const std::string &name,
            glm::u32 size,
            const BufferCreateInfo &createInfo,
            Result &resultOut
        );

        ~VulkanBuffer();

    public:
        VkBuffer GetHandle() const;

        VmaAllocation GetAllocation() const;

        glm::u32 GetSize() const;

    private:
        Result CreateBuffer(std::shared_ptr<Logger> logger);

    private:
        Renderer &m_renderer;
        std::string m_name;
        glm::u32 m_size;
        BufferCreateInfo m_createInfo;
        VkBuffer m_buffer;
        VmaAllocation m_allocation;
    };
} // Petal
