#pragma once
#include "Graphics/GraphicsContext.h"
#include "Graphics/VertexType.h"

namespace Petal {
    class VulkanGraphicsPipeline {
    public:
        struct PipelineSettings {
            bool RenderWireframe = false;
            bool CullBackFaces = true;
            glm::u32 PushConstantsSize = 128;

            Result Validate(std::shared_ptr<Logger> logger);
        };

    public:
        VulkanGraphicsPipeline(
            GraphicsContext &renderer,
            const VulkanShader &shader,
            const std::shared_ptr<Logger> &logger,
            const PipelineSettings &settings,
            Result &resultOut
        );

        ~VulkanGraphicsPipeline();

    public:
        VkPipeline GetHandle() const;

        VkPipelineLayout GetLayout() const;

    private:
        Result CreatePipelineLayout();

        Result CreatePipeline();

    private:
        GraphicsContext &m_renderer;
        const VulkanShader &m_shader;
        std::shared_ptr<Logger> m_logger;
        PipelineSettings m_settings;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_pipeline;
    };
} // Petal
