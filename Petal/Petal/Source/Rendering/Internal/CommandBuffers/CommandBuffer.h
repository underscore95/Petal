#pragma once

#include "Common.h"

namespace Petal {

class CommandBuffer {
public:
    // Create in Renderer
    CommandBuffer(
        Ref<Logger> logger,
        VkDevice device,
        VkCommandPool pool,
        VkCommandBufferLevel level,
       Result &out
        );
    ~CommandBuffer();

    CommandBuffer(CommandBuffer&& other) noexcept;
    CommandBuffer& operator=(CommandBuffer&& other) noexcept;

public:
    Result Begin(VkCommandBufferUsageFlags usageFlags) const ;

    Result End() const ;

    VkCommandBuffer GetHandle() const;

private:
    Result CreateCommandBuffer(VkCommandBufferLevel level);

private:
    Ref<Logger>m_logger;
    VkDevice m_device;
    VkCommandPool m_commandPool;
    VkCommandBuffer m_handle;
};

} // Petal