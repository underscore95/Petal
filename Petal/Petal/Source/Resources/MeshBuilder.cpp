#include "MeshBuilder.h"

namespace Petal {
    MeshBuilder::MeshBuilder(
        const VertexType &vertexType,
        const IndexType &indexType
    )
        : m_vertexType(vertexType),
          m_indexType(indexType) {
        assert(indexType == IndexType::INDICES_32_BIT); // todo Model::GetFirstVertex() needs to support mix and matching...
    }

    void MeshBuilder::PushVertices(glm::u32 numVertices, const void *data) {
        m_vertices.resize(m_vertices.size() + numVertices * m_vertexType.Size);
        char *verticesEnd = m_vertices.data() + m_numVertices * m_vertexType.Size;
        memcpy(verticesEnd, data, numVertices * m_vertexType.Size);
        m_numVertices += numVertices;
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
