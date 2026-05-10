#include "MeshBuilder.h"

namespace Petal {
    MeshBuilder::MeshBuilder(
        const VertexType &vertexType,
        const IndexType &indexType
    )
        : m_vertexType(vertexType),
          m_indexType(indexType) {
    }

    void MeshBuilder::PushVertex(
        const void *data
    ) {
        for (glm::u32 i = 0; i < m_vertexType.Size; i++) {
            const char *byte = static_cast<const char *>(data) + i;
            m_vertices.push_back(*byte);
        }
        m_numVertices++;
    }

    const MeshBuilder::VertexType &MeshBuilder::GetVertexType() const {
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
