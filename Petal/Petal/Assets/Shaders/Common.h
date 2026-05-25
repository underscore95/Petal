#ifndef PETAL_SHADER_COMMON
#define PETAL_SHADER_COMMON 1

#ifdef __cplusplus

#include <glm/glm.hpp>

namespace PetalShader {
    using float3 = glm::vec3;
    using float2 = glm::vec2;
    using float4x4 = glm::mat4x4;
    using uint = glm::u32;

#endif

    struct VertexData {
        float3 position;
        float padding;
        float3 normal;
        float padding2;
        float2 uv;
        float padding3;
        float padding4;
    };

    struct Camera {
        float4x4 mvp;
    };

    struct Params {
        uint DiffuseMapIndex;
        // what index in the vertex buffer does this meshes vertices start
        uint MeshVertexBufferStart;
    };

#ifdef __cplusplus
} // PetalShader
#endif

#endif // PETAL_SHADER_COMMON
