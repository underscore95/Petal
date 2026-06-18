#pragma once

#include "RendererSettings.h"
#include "Graphics/Internal/VulkanSwapchain.h"

namespace Petal {
    class ModelResource;
    class MeshResource;
    class VulkanGraphicsPipeline;
    class Camera;
    class VulkanShader;
    class GraphicsContext;
    class SwapchainBuffers;
    struct Params;
    class Model;
    class VulkanBuffer;
    class MeshBuilder;

    class Renderer {
    public:
        // Create in GraphicsContext
        explicit Renderer(
            GraphicsContext &context,
            const std::shared_ptr<Logger> &logger,
            const RendererSettings &rendererSettings,
            Result &outResult
        );

        ~Renderer();

    public:
        // Call this once per frame
        void Render() const;

        // Upload a mesh to the GPU
        // Optionally include a name for the resource
        AllocatedOptional<MeshResource> UploadMesh(
            const MeshBuilder &mesh,
            std::string name = ""
        );

        // Upload a model to the GPU
        // Optionally include a name for the resource.
        AllocatedOptional<ModelResource> UploadModel(
            const Model &model,
            std::string name = ""
        );

        void CmdRender(
            const VulkanSwapchain::RenderCommandBuffers &commandBuffers,
            const VulkanGraphicsPipeline &pipeline,
            const Petal::Params &params,
            // todo something about this
            const MeshResource &mesh, glm::u32 instances = 1
        ) const;

        void CmdRender(
            const VulkanSwapchain::RenderCommandBuffers &commandBuffers,
            const VulkanGraphicsPipeline &pipeline, const ModelResource &model
        ) const;

        void SetCamera(const Camera &camera);

        Result Bind(const VulkanShader &shader) const;

        GraphicsContext &GetContext() const;

    private:
        Result CreateBuffers();

    private:
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        std::shared_ptr<VulkanBuffer> m_vertexBuffer;
        std::shared_ptr<VulkanBuffer> m_indexBuffer;
        std::unique_ptr<SwapchainBuffers> m_cameraBuffers;
        glm::u32 m_numUploadedMeshes = 0;
        glm::u32 m_numUploadedModels = 0;
        RendererSettings m_rendererSettings;
        bool m_hasUploadedCamera = false;
    };
} // Petal
