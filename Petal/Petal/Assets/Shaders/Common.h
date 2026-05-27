#ifndef PETAL_SHADER_COMMON
#define PETAL_SHADER_COMMON 1

#ifdef __cplusplus

#include <glm/glm.hpp>

namespace PetalShader {
    using float3 = glm::vec3;
    using float2 = glm::vec2;
    using float4x4 = glm::mat4x4;
    using uint = glm::u32;
    using float4x4 = glm::mat4x4;

#endif

    struct VertexData {
        float3 position;
        float3 normal;
        float2 uv;
    };

    struct Params {
        float4x4 ViewMatrix;
        float4x4 ProjMatrix;
        uint DiffuseMapIndex;
        uint Padding;
        uint Padding2;
        uint Padding3;
    };

#ifdef __cplusplus
} // PetalShader
#endif

#endif // PETAL_SHADER_COMMON
