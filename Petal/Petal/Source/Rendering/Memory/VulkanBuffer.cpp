#include "VulkanBuffer.h"
#include "Rendering/Renderer.h"

namespace Petal {
    VulkanBuffer::VulkanBuffer(
        Renderer &renderer,
        const std::shared_ptr<Logger> &logger,
        const std::string &name,
        glm::u32 size,
        const BufferCreateInfo &createInfo,
        Result &resultOut
    )
        : m_renderer(renderer),
          m_name(name),
          m_size(size),
          m_createInfo(createInfo) {
        resultOut = CreateBuffer(logger);
    }

    VulkanBuffer::~VulkanBuffer() {
        vmaDestroyBuffer(m_renderer.GetAllocator()->GetAllocator(), m_buffer, m_allocation);
    }

    VkBuffer VulkanBuffer::GetHandle() const {
        return m_buffer;
    }

    VmaAllocation VulkanBuffer::GetAllocation() const {
        return m_allocation;
    }

    glm::u32 VulkanBuffer::GetSize() const {
        return m_size;
    }

    Result VulkanBuffer::CreateBuffer(std::shared_ptr<Logger> logger) {
        PETAL_CHECK_COND(m_size == 0, Result::VMA_BUFFER_CREATION_FAILED, logger, "Attempted to create buffer {} with size 0", m_name);

        VkBufferUsageFlags usage = BufferTypes::GetData(m_createInfo.BufferType).VulkanUsage;
        if (m_createInfo.IsTransferDest) usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (m_createInfo.IsTransferSource) usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
#
        VkMemoryPropertyFlags properties = 0;
        if (m_createInfo.DeviceLocal) properties |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        if (m_createInfo.HostVisible) properties |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

        VkBufferCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = m_size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo allocInfo = {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
            .requiredFlags = properties
        };

        VkResult result = vmaCreateBuffer(
            m_renderer.GetAllocator()->GetAllocator(),
            &info,
            &allocInfo,
            &m_buffer,
            &m_allocation,
            nullptr
        );

        PETAL_CHECK_COND(
            result !=VK_SUCCESS,
            Result::VMA_BUFFER_CREATION_FAILED,
            logger,
            "Failed to create Vulkan buffer with name {}: {}", m_name, result
        );

        return Result::SUCCESS;
    }
} // Petal
