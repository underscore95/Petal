#pragma once

#include <assimp/mesh.h>

#include "Common.h"
#include "Graphics/VertexType.h"
#include "Graphics/Other/IndexType.h"

namespace Petal {
    class MeshBuilder {
    public:
        MeshBuilder(
            const VertexType &vertexType,
            const IndexType &indexType
        );

    public:
        // Push vertices
        void PushVertices(glm::u32 numVertices, const void *data);

        // Push indices
        template<typename IndexType>
        void PushIndices(glm::u32 numIndices, const IndexType *indices) {
            assert(IndexTypes::GetData(m_indexType).Type == typeid(IndexType) && "Attempted to push index of wrong size to mesh");
            m_indices.resize(m_indices.size() + numIndices * sizeof(IndexType));
            void *indicesEnd = m_indices.data() + m_numIndices;
            memcpy(indicesEnd, indices, numIndices * sizeof(IndexType));
            m_numIndices += numIndices;
        }

        const VertexType &GetVertexType() const;

        const IndexType &GetIndexType() const;

        glm::u32 GetVertexBufferSize() const;

        glm::u32 GetIndexBufferSize() const;

        glm::u32 GetNumVertices() const;

        glm::u32 GetNumIndices() const;

        const void *GetVertices() const;

        const void *GetIndices() const;

    private:
        std::vector<char> m_vertices;
        VertexType m_vertexType;
        glm::u32 m_numVertices = 0;

        std::vector<char> m_indices;
        IndexType m_indexType;
        glm::u32 m_numIndices = 0;
    };
} // Petal
