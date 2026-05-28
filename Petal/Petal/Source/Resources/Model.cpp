#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Graphics/Other/IndexType.h"
#include "MeshBuilder.h"

namespace Petal {
    Model::Model(
        const std::shared_ptr<Logger> &logger,
        const std::filesystem::path &path,
        const Settings &settings,
        Result &resultOut
    ) {
        resultOut = LoadModel(logger, path, settings);
        if (resultOut != Result::SUCCESS) return;

        logger->Verbose("Loaded model {}", path);
    }

    const std::vector<Model::Section> &Model::GetSections() const {
        return m_sections;
    }

    template<typename T>
    void WriteIndices(std::vector<char> &indices, const aiMesh *mesh) {
        constexpr glm::u32 INDICES_PER_FACE = 3;

        T *dst = reinterpret_cast<T *>(indices.data());

        glm::u32 position = 0;

        for (glm::u32 faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
            const aiFace &face = mesh->mFaces[faceIndex];

            assert(face.mNumIndices == INDICES_PER_FACE);

            for (glm::u32 indexIndex = 0; indexIndex < face.mNumIndices; indexIndex++) {
                dst[position++] = static_cast<T>(face.mIndices[indexIndex]);
            }
        }
    }

    Result Model::LoadModel(
        const std::shared_ptr<Logger> &logger,
        const std::filesystem::path &path,
        const Settings &settings // todo actually use vertex type (like don't push normals if the vertex type doesn't want them etc)
    ) {
        glm::u32 flags = aiProcess_Triangulate;

        Assimp::Importer importer;
        const std::string pathString = path.string();
        const aiScene *scene = importer.ReadFile(pathString.c_str(), flags);
        PETAL_CHECK_COND(!scene, Result::ASSIMP_MODEL_LOADING_FAILED, logger, "aiScene was nullptr");
        PETAL_CHECK_COND((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0, Result::ASSIMP_MODEL_LOADING_FAILED, logger, "aiScene was incomplete");
        PETAL_CHECK_COND(!scene->mRootNode, Result::ASSIMP_MODEL_LOADING_FAILED, logger, "No root node in aiScene");

        m_sections.reserve(scene->mNumMeshes);
        for (glm::u32 meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
            const aiMesh *mesh = scene->mMeshes[meshIndex];
            const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            glm::u32 numDiffuseTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
            aiString materialName = material->GetName();

            PETAL_CHECK_COND(!scene->mMeshes[meshIndex]->HasTextureCoords(0), Result::PETAL_MODEL_LOADING_FAILED, logger, "No channel 0 texture coordinates");

            // Get diffuse texture
            PETAL_CHECK_COND(
                numDiffuseTextures != 1,
                Result::PETAL_MODEL_LOADING_FAILED,
                logger,
                "Material {} has {} (not 1) diffuse textures", materialName.C_Str(), numDiffuseTextures
            );

            aiString diffuseTexture;
            aiReturn result = material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexture); // todo support embedded textures
            PETAL_CHECK_COND(result == aiReturn_FAILURE, Result::ASSIMP_MODEL_LOADING_FAILED, logger, "Failed to get diffuse texture of mesh {}", materialName.C_Str());

            // indices
            constexpr glm::u32 INDICES_PER_FACE = 3;
            const glm::u32 numIndices = mesh->mNumFaces * INDICES_PER_FACE;

            IndexType indexType = IndexTypes::GetBestIndexType(mesh->mNumVertices);
            const IndexTypes::EnumData &indexTypeData = IndexTypes::GetData(indexType);

            logger->Verbose("Selected IndexType {} for {} vertices", indexType, mesh->mNumVertices);

            std::vector<char> indices;
            indices.resize(numIndices * indexTypeData.SizeInBytes);

            switch (indexType) {
                case IndexType::INDICES_16_BIT:
                    WriteIndices<glm::u16>(indices, mesh);
                    break;
                case IndexType::INDICES_32_BIT:
                    WriteIndices<glm::u32>(indices, mesh);
                    break;
                default:
                    PETAL_ERROR(Result::ASSIMP_MODEL_LOADING_FAILED, logger, "IndexType {} was selected but not supported!", indexType);
            }

            // vertices
            std::vector<float> vertices;
            for (glm::u32 vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) {
                // Position
                vertices.push_back(mesh->mVertices[vertexIndex].x);
                vertices.push_back(mesh->mVertices[vertexIndex].y);
                vertices.push_back(mesh->mVertices[vertexIndex].z);

                // Normal
                vertices.push_back(mesh->mNormals[vertexIndex].x);
                vertices.push_back(mesh->mNormals[vertexIndex].y);
                vertices.push_back(mesh->mNormals[vertexIndex].z);

                // Texture coordinates
                vertices.push_back(mesh->mTextureCoords[0][vertexIndex].x);
                vertices.push_back(mesh->mTextureCoords[0][vertexIndex].y);
            }

            // Create mesh
            Section section = {
                .Mesh = std::make_unique<MeshBuilder>(settings.VertexType, indexType),
                .DiffuseTexture = diffuseTexture.C_Str()
            };

            section.Mesh->PushVertices(mesh->mNumVertices, vertices.data());
            section.Mesh->SetIndices(indices.data(), indices.size(), indexType);

            m_sections.push_back(std::move(section));
        }

        return Result::SUCCESS;
    }
} // Petal
