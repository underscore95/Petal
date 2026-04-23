#include "VulkanQueue.h"

#include "RenderingDevice.h"
#include "Rendering/Renderer.h"

namespace Petal {
    VulkanQueue::VulkanQueue(
        Renderer &renderer,
        std::shared_ptr<Logger> logger,
        glm::u32 queueFamily
    ) : m_renderer(renderer),
        m_logger(logger),
        m_queueFamily(queueFamily) {
        vkGetDeviceQueue(m_renderer.GetDevice()->GetDevice(), queueFamily, 0, &m_handle);

        logger->Verbose("VulkanQueue initialized");
    }

    VulkanQueue::~VulkanQueue() {
    }

    glm::u32 VulkanQueue::GetQueueFamilyIndex() const {
        return m_queueFamily;
    }

    VkQueue VulkanQueue::GetHandle() const {
        return m_handle;
    }
} // Petal
