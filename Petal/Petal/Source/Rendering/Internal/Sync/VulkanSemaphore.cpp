#include "VulkanSemaphore.h"

#include "Rendering/Renderer.h"
#include "Rendering/Internal/RenderingDevice.h"

namespace Petal {
    VulkanSemaphore::VulkanSemaphore(
        Renderer &renderer,
        Ref<Logger> logger,
        Result &out
    ) : m_renderer(renderer) {
        out = CreateSemaphore(logger);
    }

    VulkanSemaphore::~VulkanSemaphore() {
        if (m_handle) vkDestroySemaphore(m_renderer.GetDevice()->GetDevice(), m_handle, nullptr);
    }

    VkSemaphore VulkanSemaphore::GetHandle() const {
        return m_handle;
    }

    Result VulkanSemaphore::CreateSemaphore(Ref<Logger> logger) {
        VkSemaphoreCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };

        VkResult res = vkCreateSemaphore(m_renderer.GetDevice()->GetDevice(), &info, nullptr, &m_handle);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_SEMAPHORE_CREATION_FAILED, logger, "{}", res);
        return Result::SUCCESS;
    }
}
