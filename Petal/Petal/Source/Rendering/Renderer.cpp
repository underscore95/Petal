#include "Renderer.h"

#include "MeshBuilder.h"
#include "Graphics/Internal/VulkanShader.h"
#include "Graphics/Memory/GPUBufferSubsystem.h"

namespace Petal {
    Renderer::Renderer(
        GraphicsContext &context,
        const std::shared_ptr<Logger> &logger,
        const RendererSettings &rendererSettings,
        Result &outResult
    ) : m_context(context),
        m_logger(logger),
        m_rendererSettings(rendererSettings) {
        outResult = CreateBuffers();
        if (outResult != Result::SUCCESS) return;
    }

    Renderer::~Renderer() {
    }

    AllocatedOptional<MeshResource> Renderer::UploadMesh(
        const MeshBuilder &mesh
    ) {
        std::string name = std::format("Mesh {}", m_numUploadedMeshes);
        m_numUploadedMeshes++;

        AllocatedOptional<GPUBuffer> vertexBuffer = m_context.GetBufferSubsystem().CreateBackedBuffer(
            name + " Vertex Buffer",
            mesh.GetVertexBufferSize(),
            m_vertexBuffer
        );
        PETAL_CHECK_OPTIONAL_SILENT(vertexBuffer);
        m_context.GetBufferSubsystem().Write(*vertexBuffer.Value(), mesh.GetVertices(), mesh.GetVertexBufferSize());

        AllocatedOptional<GPUBuffer> indexBuffer = m_context.GetBufferSubsystem().CreateBackedBuffer(
            name + " Index Buffer",
            mesh.GetIndexBufferSize(),
            m_indexBuffer
        );
        PETAL_CHECK_OPTIONAL_SILENT(indexBuffer);
        m_context.GetBufferSubsystem().Write(*indexBuffer.Value(), mesh.GetIndices(), mesh.GetIndexBufferSize());

        auto meshResource = std::make_unique<MeshResource>(m_context, m_logger, vertexBuffer.Release(), indexBuffer.Release(), mesh.GetNumIndices());
        return meshResource;
    }

    Result Renderer::Bind(VulkanShader &shader) const {
        Result result = shader.BindBuffer(m_rendererSettings.VertexBufferShaderName, *m_vertexBuffer);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
        result = shader.BindBuffer(m_rendererSettings.IndexBufferShaderName, *m_indexBuffer);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
        return Result::SUCCESS;
    }

    Result Renderer::CreateBuffers() {
        constexpr glm::u32 VERTEX_BUFFER_SIZE = 1024 * 1024 * 512;
        AllocatedOptional<VulkanBuffer> bufferOpt = m_context.GetBufferSubsystem().CreateVulkanBuffer(
            "Vertex Buffer",
            VERTEX_BUFFER_SIZE
        );
        PETAL_CHECK_OPTIONAL(bufferOpt, m_logger, "Failed to create vertex buffer");
        m_vertexBuffer = bufferOpt.Release();

        constexpr glm::u32 INDEX_BUFFER_SIZE = 1024 * 1024 * 64;
        bufferOpt = m_context.GetBufferSubsystem().CreateVulkanBuffer(
            "Index Buffer",
            INDEX_BUFFER_SIZE
        );
        PETAL_CHECK_OPTIONAL(bufferOpt, m_logger, "Failed to create index buffer");
        m_indexBuffer = bufferOpt.Release();

        return Result::SUCCESS;
    }
} // Petal
