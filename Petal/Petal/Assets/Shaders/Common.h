#ifndef PETAL_GUARD_COMMON
#define PETAL_GUARD_COMMON 1

#include "Petal/Math.h"

#ifdef __cplusplus
namespace Petal {
#endif

    struct VertexData {
        float3 position;
        float3 normal;
        float2 uv;
    };

    struct Params {
        uint DiffuseMap;
    };

    struct CameraMatrices {
        float4x4 ViewMatrix;
        float4x4 ProjMatrix;
    };

#ifdef __cplusplus
} // PetalShader
#endif

#endif // PETAL_GUARD_COMMON
