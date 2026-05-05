#include "VulkanFence.h"

#include "Graphics/GraphicsContext.h"
#include "Graphics/Internal/RenderingDevice.h"

namespace Petal {
    VulkanFence::VulkanFence(
        GraphicsContext &renderer,
        VkFenceCreateFlags flags,
        std::shared_ptr<Logger> logger,
        Result &out
    ) : m_renderer(renderer) {
        out = CreateFence(logger, flags);
    }

    VulkanFence::~VulkanFence() {
        if (m_handle != VK_NULL_HANDLE) vkDestroyFence(m_renderer.GetDevice()->GetDevice(), m_handle, nullptr);
    }

    VkFence VulkanFence::GetHandle() const {
        return m_handle;
    }

    Result VulkanFence::CreateFence(std::shared_ptr<Logger> logger, VkFenceCreateFlags flags) {
        VkFenceCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags
        };

        VkResult res = vkCreateFence(m_renderer.GetDevice()->GetDevice(), &info, nullptr, &m_handle);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_FENCE_CREATION_FAILED, logger, "{}", res);

        return Result::SUCCESS;
    }
} // Petal
