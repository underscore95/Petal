#pragma once

#include "Common.h"
#include "../../Graphics/Memory/Buffers/GPUBuffer.h"
#include "Graphics/Other/IndexType.h"

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
            glm::u32 vertexSize,
            IndexType indexType
        );

    public:
        glm::u32 GetNumIndices() const;

        const GPUBuffer &GetVertexBuffer() const;

        const GPUBuffer &GetIndexBuffer() const;

        IndexType GetIndexType() const;

    private:
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        std::unique_ptr<GPUBuffer> m_vertexBuffer;
        std::unique_ptr<GPUBuffer> m_indexBuffer;
        glm::u32 m_numIndices;
        glm::u32 m_vertexSize;
        IndexType m_indexType;
    };
} // Petal
