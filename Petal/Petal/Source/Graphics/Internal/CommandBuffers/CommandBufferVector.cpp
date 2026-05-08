#include "CommandBufferVector.h"

#include "Graphics/GraphicsContext.h"
#include "Graphics/Internal/RenderingDevice.h"
#include "Graphics/Internal/VulkanSwapchain.h"

namespace Petal {
    CommandBufferVector::CommandBufferVector(
        std::shared_ptr<Logger> logger,
        GraphicsContext &context,
        VkCommandPool pool,
        VkCommandBufferLevel level,
        glm::u32 count,
        Result &out
    ) : m_logger(logger),
        m_context(context),
        m_commandPool(pool) {
        out = CreateCommandBuffers(level, count);
    }

    CommandBufferVector::~CommandBufferVector() {
        if (!m_handles.empty()) {
            vkFreeCommandBuffers(
                m_context.GetDevice()->GetDevice(),
                m_commandPool,
                static_cast<glm::u32>(m_handles.size()),
                m_handles.data()
            );
        }
    }

    CommandBufferVector::CommandBufferVector(CommandBufferVector &&other) noexcept
        : m_logger(std::move(other.m_logger)),
          m_context(other.m_context),
          m_commandPool(other.m_commandPool),
          m_handles(std::move(other.m_handles)) {
        other.m_handles.clear();
    }

    Result CommandBufferVector::Begin(glm::u32 index, VkCommandBufferUsageFlags usageFlags) const {
        PETAL_CHECK_COND(index >= m_handles.size(), Result::VULKAN_COMMAND_BUFFER_BEGIN_FAILED, m_logger, "Index out of bounds: {}", index);

        VkCommandBufferBeginInfo info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = usageFlags,
            .pInheritanceInfo = nullptr
        };

        VkResult res = vkBeginCommandBuffer(m_handles[index], &info);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_COMMAND_BUFFER_BEGIN_FAILED, m_logger, "{}", res);
        return Result::SUCCESS;
    }

    Result CommandBufferVector::End(glm::u32 index) const {
        PETAL_CHECK_COND(index >= m_handles.size(), Result::VULKAN_COMMAND_BUFFER_END_FAILED, m_logger, "Index out of bounds: {} (size {})", index, m_handles.size());

        VkResult res = vkEndCommandBuffer(m_handles[index]);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_COMMAND_BUFFER_END_FAILED, m_logger, "{}", res);
        return Result::SUCCESS;
    }

    Result CommandBufferVector::BeginAll(VkCommandBufferUsageFlags usageFlags) const {
        for (glm::u32 i = 0; i < Size(); i++) {
            Result result = Begin(i, usageFlags);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
        }
        return Result::SUCCESS;
    }

    Result CommandBufferVector::EndAll() const {
        for (glm::u32 i = 0; i < Size(); i++) {
            Result result = End(i);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
        }
        return Result::SUCCESS;
    }

    VkCommandBuffer CommandBufferVector::GetHandle(glm::u32 index) const {
        return m_handles[index];
    }

    glm::u32 CommandBufferVector::Size() const {
        return static_cast<glm::u32>(m_handles.size());
    }

    bool CommandBufferVector::IsSwapchainSize() const {
        return m_context.GetSwapchain().NumSwapchainImages() == Size();
    }

    Result CommandBufferVector::CreateCommandBuffers(VkCommandBufferLevel level, glm::u32 count) {
        m_handles.resize(count);

        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_commandPool,
            .level = level,
            .commandBufferCount = count
        };

        VkResult res = vkAllocateCommandBuffers(m_context.GetDevice()->GetDevice(), &allocInfo, m_handles.data());
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_COMMAND_BUFFER_CREATION_FAILED, m_logger, "Failed to create command buffers because: {}", res);
        return Result::SUCCESS;
    }
}
