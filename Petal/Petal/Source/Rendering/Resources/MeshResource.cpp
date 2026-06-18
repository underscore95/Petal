#include "MeshResource.h"
#include "Graphics/Memory/Buffers/GPUBuffer.h"
#include "Graphics/Other/IndexType.h"

namespace Petal {
    static glm::u32 s_vertexSize = 0;

    MeshResource::MeshResource(
        GraphicsContext &context,
        std::shared_ptr<Logger> logger,
        std::unique_ptr<GPUBuffer> vertexBuffer,
        std::unique_ptr<GPUBuffer> indexBuffer,
        glm::u32 numIndices,
        glm::u32 vertexSize,
        IndexType indexType
    ) : m_context(context),
        m_logger(logger),
        m_vertexBuffer(std::move(vertexBuffer)),
        m_indexBuffer(std::move(indexBuffer)),
        m_numIndices(numIndices),
        m_vertexSize(vertexSize),
        m_indexType(indexType) {
        assert(s_vertexSize == 0 || m_vertexSize == s_vertexSize); // todo support varying vertex sizes
        s_vertexSize = m_vertexSize;
    }

    glm::u32 MeshResource::GetNumIndices() const {
        return m_numIndices;
    }

    const GPUBuffer &MeshResource::GetVertexBuffer() const {
        return *m_vertexBuffer;
    }

    const GPUBuffer &MeshResource::GetIndexBuffer() const {
        return *m_indexBuffer;
    }

    IndexType MeshResource::GetIndexType() const {
        return m_indexType;
    }
} // Petal
