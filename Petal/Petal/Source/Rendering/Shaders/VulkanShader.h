#pragma once

#include "Common.h"
#include <vulkan/vulkan_core.h>

#include "Rendering/Renderer.h"

namespace Petal {
    struct IntermediateShaderResource;

    class VulkanShader {
    public:
        VulkanShader(
            Renderer &renderer,
            const IntermediateShaderResource &shader,
            std::shared_ptr<Logger> logger,
            Result &resultOut
        );

        ~VulkanShader();

    public:
        VkShaderModule GetHandle() const;

    private:
        Result CreateShaderModule(
            const IntermediateShaderResource &shader,
            std::shared_ptr<Logger> logger
        );

    private:
        Renderer &m_renderer;
        VkShaderModule m_handle;
    };
} // Petal
