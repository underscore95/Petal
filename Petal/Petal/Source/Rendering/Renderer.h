#pragma once
#include "Resources/MeshResource.h"
#include "RendererSettings.h"
#include "Camera/Camera.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Memory/Textures/VulkanTexture.h"
#include "Resources/ModelResource.h"

namespace Petal {
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
            const CommandBufferVector &commandBuffers,
            const VulkanShader &shader,
            const Petal::Params &params, // todo something about this
            const MeshResource &mesh, glm::u32 instances = 1
        ) const;

        void CmdRender(
            const CommandBufferVector &commandBuffers,
            const VulkanShader &shader,
            const ModelResource &model
        ) const;

        void SetCamera(const Camera& camera);

        Result Bind(const VulkanShader &shader) const;

    private:
        Result CreateBuffers();

    private:
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        std::shared_ptr<VulkanBuffer> m_vertexBuffer;
        std::shared_ptr<VulkanBuffer> m_indexBuffer;
        std::shared_ptr<VulkanBuffer> m_cameraBuffer;
        glm::u32 m_numUploadedMeshes = 0;
        glm::u32 m_numUploadedModels = 0;
        RendererSettings m_rendererSettings;
        bool m_hasUploadedCamera = false;
    };
} // Petal
