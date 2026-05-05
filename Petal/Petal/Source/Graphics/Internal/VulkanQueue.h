#pragma once

namespace Petal {
    class GraphicsContext;

    class VulkanQueue {
    public:
        VulkanQueue(
            GraphicsContext &renderer,
            std::shared_ptr<Logger> logger,
            glm::u32 queueFamily
        );

        ~VulkanQueue();

    public:
        glm::u32 GetQueueFamilyIndex() const;

        VkQueue GetHandle() const;

    private:
        GraphicsContext &m_renderer;
        std::shared_ptr<Logger> m_logger;
        glm::u32 m_queueFamily;
        VkQueue m_handle;
    };
} // Petal
