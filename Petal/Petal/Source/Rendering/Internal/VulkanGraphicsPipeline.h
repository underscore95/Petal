#pragma once
#include "Rendering/Renderer.h"

namespace Petal {
    class VulkanGraphicsPipeline {
    public:
        struct PipelineSettings {
            bool RenderWireframe = false;
            bool CullBackFaces = true;
        };

    public:
        VulkanGraphicsPipeline(
            Renderer &renderer,
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
        Renderer &m_renderer;
        const VulkanShader &m_shader;
        std::shared_ptr<Logger> m_logger;
        PipelineSettings m_settings;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_pipeline;
    };
} // Petal
