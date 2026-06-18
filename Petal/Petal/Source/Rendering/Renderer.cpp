#include "Renderer.h"

#include "Resources/MeshBuilder.h"
#include "Graphics/Internal/VulkanShader.h"
#include "Graphics/Memory/GPUMemorySubsystem.h"
#include "../Assets/Shaders/Common.h"
#include "Resources/Model.h"
#include "Camera/Camera.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Memory/Buffers/SwapchainBuffers.h"
#include "Graphics/Memory/Buffers/GPUBuffer.h"
#include "Graphics/Memory/Buffers/VulkanBuffer.h"
#include "Resources/MeshResource.h"
#include "Resources/ModelResource.h"

namespace Petal {
    Renderer::Renderer(
        GraphicsContext &context,
        const std::shared_ptr<Logger> &logger,
        const RendererSettings &rendererSettings,
        Result &outResult
    ) : m_context(context),
        m_logger(logger),
        m_rendererSettings(rendererSettings) {
        outResult = m_rendererSettings.Validate(m_logger);
        if (outResult != Result::SUCCESS) return;

        outResult = CreateBuffers();
        if (outResult != Result::SUCCESS) return;
    }

    Renderer::~Renderer() {
    }

    void Renderer::Render() const {
        m_cameraBuffers->Render();
    }

    AllocatedOptional<MeshResource> Renderer::UploadMesh(
        const MeshBuilder &mesh,
        std::string name
    ) {
        if (name.empty()) {
            name = std::format("Mesh {}", m_numUploadedMeshes);
        }
        m_numUploadedMeshes++;

        AllocatedOptional<GPUBuffer> vertexBuffer = m_context.GetMemorySubsystem().CreateBackedBuffer(
            name + " Vertex Buffer",
            mesh.GetVertexBufferSize(),
            m_vertexBuffer
        );
        PETAL_CHECK_OPTIONAL_SILENT(vertexBuffer);
        m_context.GetMemorySubsystem().Write(*vertexBuffer.Value(), mesh.GetVertices(), mesh.GetVertexBufferSize());

        AllocatedOptional<GPUBuffer> indexBuffer = m_context.GetMemorySubsystem().CreateBackedBuffer(
            name + " Index Buffer",
            mesh.GetIndexBufferSize(),
            m_indexBuffer
        );
        PETAL_CHECK_OPTIONAL_SILENT(indexBuffer);
        m_context.GetMemorySubsystem().Write(*indexBuffer.Value(), mesh.GetIndices(), mesh.GetIndexBufferSize());

        auto meshResource = std::make_unique<MeshResource>(
            m_context,
            m_logger,
            vertexBuffer.Release(),
            indexBuffer.Release(),
            mesh.GetNumIndices(),
            mesh.GetVertexType().Size,
            mesh.GetIndexType()
        );
        return meshResource;
    }

    AllocatedOptional<ModelResource> Renderer::UploadModel(
        const Model &model,
        std::string name
    ) {
        if (name.empty()) {
            name = std::format("Model {}", m_numUploadedModels);
        }

        std::vector<std::unique_ptr<MeshResource> > meshes;
        meshes.reserve(model.GetSections().size());

        glm::u32 meshIndex = 0;
        for (const Model::Section &section : model.GetSections()) {
            AllocatedOptional<MeshResource> mesh = UploadMesh(*section.Mesh, std::format("{} (Mesh {})", name, meshIndex));
            PETAL_CHECK_OPTIONAL(mesh, m_logger, "Failed to upload mesh {} of model {}", meshIndex, name);

            meshIndex++;
            meshes.push_back(mesh.Release());
        }

        m_logger->Info("Uploaded model {}", name);
        return std::make_unique<ModelResource>(m_logger, model, std::move(meshes));
    }

    void Renderer::CmdRender(
        const VulkanSwapchain::RenderCommandBuffers &commandBuffers,
        const VulkanGraphicsPipeline &pipeline,
        const Petal::Params &params,
        const MeshResource &mesh,
        glm::u32 instances
    ) const {
        if (!m_hasUploadedCamera) [[unlikely]] {
            m_logger->Error("You must call SetCamera before rendering a mesh.");
        }

        pipeline.GetShader().CmdBindResources(pipeline, commandBuffers.GetCommands());

        m_context.CmdBindVertexBuffer(commandBuffers.GetCommands(), 0, {std::cref(mesh.GetVertexBuffer())});
        m_context.CmdBindIndexBuffer(commandBuffers.GetCommands(), mesh.GetIndexBuffer(), mesh.GetIndexType());

        m_context.CmdWritePushConstants(commandBuffers.GetCommands(), pipeline, &params, sizeof(params));

        m_context.GetSwapchain().CmdRenderIndexed(pipeline, commandBuffers, mesh.GetNumIndices(), instances);
    }

    void Renderer::CmdRender(
        const VulkanSwapchain::RenderCommandBuffers &commandBuffers,
        const VulkanGraphicsPipeline &pipeline,
        const ModelResource &model
    ) const {
        for (const std::unique_ptr<MeshResource> &mesh : model.GetMeshes()) {
            Petal::Params params = {
                .DiffuseMap = 1, // todo
            };
            CmdRender(commandBuffers, pipeline, params, *mesh);
        }
    }

    void Renderer::SetCamera(const Camera &camera) {
        m_hasUploadedCamera = true;

        Petal::CameraMatrices matrices = {
            .ViewMatrix = camera.GetViewMatrix(),
            .ProjMatrix = camera.GetProjMatrix()
        };

        m_context.GetMemorySubsystem().Write(
            (*m_cameraBuffers)[m_context.GetSwapchain().GetSwapchainIndex()],
            &matrices,
            sizeof(matrices)
        ); // TODO write async
    }

    Result Renderer::Bind(const VulkanShader &shader) const {
        Result result = shader.BindBuffer(m_rendererSettings.CameraBufferName, *m_cameraBuffers);
        if (result != Result::SUCCESS) return result;

        return result;
    }

    GraphicsContext &Renderer::GetContext() const {
        return m_context;
    }

    Result Renderer::CreateBuffers() {
        // Vertex
        constexpr glm::u32 VERTEX_BUFFER_SIZE = 1024 * 1024 * 512;
        BufferCreateInfo bufferInfo = {.BufferType = BufferType::VERTEX_BUFFER};
        AllocatedOptional<VulkanBuffer> bufferOpt = m_context.GetMemorySubsystem().CreateVulkanBuffer(
            "Vertex Buffer",
            VERTEX_BUFFER_SIZE,
            bufferInfo
        );
        PETAL_CHECK_OPTIONAL(bufferOpt, m_logger, "Failed to create vertex buffer");
        m_vertexBuffer = bufferOpt.Release();

        // Index
        constexpr glm::u32 INDEX_BUFFER_SIZE = 1024 * 1024 * 64;
        bufferInfo = {.BufferType = BufferType::INDEX_BUFFER};
        bufferOpt = m_context.GetMemorySubsystem().CreateVulkanBuffer(
            "Index Buffer",
            INDEX_BUFFER_SIZE,
            bufferInfo
        );
        PETAL_CHECK_OPTIONAL(bufferOpt, m_logger, "Failed to create index buffer");
        m_indexBuffer = bufferOpt.Release();

        // Camera
        bufferInfo = {.BufferType = BufferType::CONSTANT_BUFFER};
        AllocatedOptional<SwapchainBuffers> multipleBuffersOpt = m_context.GetMemorySubsystem().CreateSwapchainBuffers(
            "Camera Buffer",
            sizeof(Petal::CameraMatrices),
            bufferInfo
        );
        PETAL_CHECK_OPTIONAL(multipleBuffersOpt, m_logger, "Failed to create camera buffer");
        m_cameraBuffers = multipleBuffersOpt.Release();

        return Result::SUCCESS;
    }
} // Petal
