#include "MeshBuilder.h"

namespace Petal {
    MeshBuilder::MeshBuilder(
        const VertexType &vertexType,
        const IndexType &indexType
    )
        : m_vertexType(vertexType),
          m_indexType(indexType) {
    }

    void MeshBuilder::PushVertices(glm::u32 numVertices, const void *data) {
        m_vertices.resize(m_vertices.size() + numVertices * m_vertexType.Size);
        char *verticesEnd = m_vertices.data() + m_numVertices * m_vertexType.Size;
        memcpy(verticesEnd, data, numVertices * m_vertexType.Size);
        m_numVertices += numVertices;
    }

    void MeshBuilder::SetIndices(const void *indices, glm::u32 size, IndexType indexType) {
        assert(m_indexType == indexType && "Attempted to push index of wrong size to mesh");
        assert(size % IndexTypes::GetData(indexType).SizeInBytes == 0);
        m_indices.resize(size);
        memcpy(m_indices.data(), indices, size);
        m_numIndices = size / IndexTypes::GetData(indexType).SizeInBytes;
    }

    const VertexType &MeshBuilder::GetVertexType() const {
        return m_vertexType;
    }

    const IndexType &MeshBuilder::GetIndexType() const {
        return m_indexType;
    }

    glm::u32 MeshBuilder::GetVertexBufferSize() const {
        return m_vertexType.Size * m_numVertices;
    }

    glm::u32 MeshBuilder::GetIndexBufferSize() const {
        return IndexTypes::GetData(m_indexType).SizeInBytes * m_numIndices;
    }

    glm::u32 MeshBuilder::GetNumVertices() const {
        return m_numVertices;
    }

    glm::u32 MeshBuilder::GetNumIndices() const {
        return m_numIndices;
    }

    const void *MeshBuilder::GetVertices() const {
        return m_vertices.data();
    }

    const void *MeshBuilder::GetIndices() const {
        return m_indices.data();
    }
} // Petal
