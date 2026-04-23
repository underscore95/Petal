#pragma once

namespace Petal {
    class Renderer;

    class VulkanQueue {
    public:
        VulkanQueue(
            Renderer &renderer,
            std::shared_ptr<Logger> logger,
            glm::u32 queueFamily
        );

        ~VulkanQueue();

    public:
        glm::u32 GetQueueFamilyIndex() const;

        VkQueue GetHandle() const;

    private:
        Renderer &m_renderer;
        std::shared_ptr<Logger> m_logger;
        glm::u32 m_queueFamily;
        VkQueue m_handle;
    };
} // Petal
