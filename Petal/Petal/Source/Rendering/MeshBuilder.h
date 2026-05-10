#pragma once

#include "Common.h"
#include "Graphics/Other/IndexType.h"

namespace Petal {
    class MeshBuilder {
    public:
        struct VertexType {
            glm::u32 Size;
        };

    public:
        MeshBuilder(
            const VertexType &vertexType,
            const IndexType &indexType
        );

    public:
        // Push a vertex
        // It is recommended to upload a vector of vertices instead as this is slow.
        void PushVertex(const void *data);

        // Push an index.
        // It is recommended to upload a vector of indices instead as this is slow.
        template<typename IndexType>
        void PushIndex(IndexType index) {
            assert(IndexTypes::GetData(m_indexType).Type == typeid(IndexType) && "Attempted to push index of wrong size to mesh");
            for (glm::u32 i = 0; i < sizeof(index); i++) {
                const char *byte = reinterpret_cast<const char *>(&index) + i;
                m_indices.push_back(*byte);
            }
            m_numIndices++;
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
