#include "CommandBuffer.h"

namespace Petal {
    CommandBuffer::CommandBuffer(
        std::shared_ptr<Logger> logger,
        VkDevice device,
        VkCommandPool pool,
        VkCommandBufferLevel level,
        Result &out
    ) : m_logger(logger),
        m_device(device),
        m_commandPool(pool) {
        out = CreateCommandBuffer(level);
    }

    CommandBuffer::~CommandBuffer() {
        if (m_handle) { vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_handle); }
    }

    CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept
        : m_logger(std::move(other.m_logger)),
          m_device(other.m_device),
          m_commandPool(other.m_commandPool),
          m_handle(other.m_handle) {
        other.m_handle = VK_NULL_HANDLE;
    }

    CommandBuffer &CommandBuffer::operator=(CommandBuffer &&other) noexcept {
        if (this != &other) {
            if (m_handle) {
                vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_handle);
            }

            m_logger = std::move(other.m_logger);
            m_device = other.m_device;
            m_commandPool = other.m_commandPool;
            m_handle = other.m_handle;

            other.m_handle = VK_NULL_HANDLE;
        }
        return *this;
    }

    Result CommandBuffer::Begin(VkCommandBufferUsageFlags usageFlags) const {
        VkCommandBufferBeginInfo info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = usageFlags,
            .pInheritanceInfo = nullptr
        };
        VkResult res = vkBeginCommandBuffer(m_handle, &info);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_COMMAND_BUFFER_BEGIN_FAILED, m_logger, "{}", res);
        return Result::SUCCESS;
    }

    Result CommandBuffer::End() const {
        VkResult res = vkEndCommandBuffer(m_handle);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_COMMAND_BUFFER_END_FAILED, m_logger, "{}", res);
        return Result::SUCCESS;
    }

    VkCommandBuffer CommandBuffer::GetHandle() const {
        return m_handle;
    }

    Result CommandBuffer::CreateCommandBuffer(VkCommandBufferLevel level) {
        VkCommandBufferAllocateInfo cmdBufAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_commandPool,
            .level = level,
            .commandBufferCount = 1
        };

        VkResult res = vkAllocateCommandBuffers(m_device, &cmdBufAllocInfo, &m_handle);

        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_COMMAND_BUFFER_CREATION_FAILED, m_logger, "Failed to create command buffer because: {}", res);
        return Result::SUCCESS;
    }
} // Petal
