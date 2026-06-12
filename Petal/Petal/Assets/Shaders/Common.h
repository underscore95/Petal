#ifndef PETAL_GUARD_COMMON
#define PETAL_GUARD_COMMON 1

#include "Petal/Math.h"
#include "Petal/Alignment.h"

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

    struct Light {
        float3 Position;
        UINT_PADDING_1;
        float3 Color;
        UINT_PADDING_1;
    };

    struct DeferredLighting {
        Light Lights[1024];

        uint NumLights;
        UINT_PADDING_3;

        UINT_PADDING_4;
        UINT_PADDING_4;
        UINT_PADDING_4;
    };

    struct CameraMatrices {
        float4x4 ViewMatrix;
        float4x4 ProjMatrix;
    };

#ifdef __cplusplus
} // PetalShader
#endif

#endif // PETAL_GUARD_COMMON
