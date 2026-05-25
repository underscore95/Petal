#pragma once

#include "Common.h"
#include "../../Graphics/Memory/Buffers/GPUBuffer.h"

namespace Petal {
    class Renderer;
    class GraphicsContext;

    class MeshResource {
        struct Settings {
        };

    public:
        MeshResource(
            GraphicsContext &context,
            std::shared_ptr<Logger> logger,
            std::unique_ptr<GPUBuffer> vertexBuffer,
            std::unique_ptr<GPUBuffer> indexBuffer,
            glm::u32 numIndices,
            glm::u32 vertexSize
        );

    public:
        glm::u32 GetNumIndices() const;

        // Index of the first index in the index buffer... indices[GetFirstIndex()] is the first index to render
        glm::u32 GetFirstIndex() const;

        // Index of the first vertex in the vertex buffer... vertices[GetFirstVertex()] is the first vertex to render
        glm::u32 GetFirstVertex() const;

    private:
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        std::unique_ptr<GPUBuffer> m_vertexBuffer;
        std::unique_ptr<GPUBuffer> m_indexBuffer;
        glm::u32 m_numIndices;
        glm::u32 m_vertexSize;
    };
} // Petal
