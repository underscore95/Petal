#include "VulkanShader.h"

#include "IntermediateShaderResource.h"
#include "Rendering/Internal/RenderingDevice.h"

namespace Petal {
    VulkanShader::VulkanShader(
        Renderer &renderer,
        const IntermediateShaderResource &shader,
        std::shared_ptr<Logger> logger,
        Result &resultOut
    ) : m_renderer(renderer) {
        resultOut = CreateShaderModule(shader, logger);
        if (resultOut != Result::SUCCESS)return;

        logger->Verbose("Compiled shader!");
    }

    VulkanShader::~VulkanShader() {
        if (m_handle) vkDestroyShaderModule(m_renderer.GetDevice()->GetDevice(), m_handle, nullptr);
    }

    Result VulkanShader::CreateShaderModule(
        const IntermediateShaderResource &shader,
        std::shared_ptr<Logger> logger
    ) {
        VkShaderModuleCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = shader.SPIRV->getBufferSize(), // size in bytes even though the ptr is u32
            .pCode = static_cast<const glm::u32 *>(shader.SPIRV->getBufferPointer())
        };

        VkResult result = vkCreateShaderModule(
            m_renderer.GetDevice()->GetDevice(),
            &info,
            nullptr,
            &m_handle
        );
        PETAL_CHECK_COND(
            result !=VK_SUCCESS,
            Result::VULKAN_SHADER_CREATION_FAILED,
            logger,
            "Failed to create shader module: {}", result
        );
        return Result::SUCCESS;
    }
} // Petal
