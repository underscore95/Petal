#include "MeshResource.h"
#include "../Graphics/Memory/Buffers/GPUBuffer.h"

namespace Petal {
    MeshResource::MeshResource(
        GraphicsContext &context,
        std::shared_ptr<Logger> logger,
        std::unique_ptr<GPUBuffer> vertexBuffer,
        std::unique_ptr<GPUBuffer> indexBuffer,
        glm::u32 numIndices
    ) : m_context(context),
        m_logger(logger),
        m_vertexBuffer(std::move(vertexBuffer)),
        m_indexBuffer(std::move(indexBuffer)),
        m_numIndices(numIndices) {
    }

    glm::u32 MeshResource::GetNumIndices() const {
        return m_numIndices;
    }
} // Petal
