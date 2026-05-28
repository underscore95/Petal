// http://shader-slang.org/slang-playground/
// This example demonstrates how to define vertex and fragment shader entrypoints.
// This is a compile-only example, so there is no output from the playground.
// You can use the "Compile" button to compile this example to different targets.

#include "Common.h"

Sampler2D textures[];
//uniform CameraMatrices camera;
StructuredBuffer<CameraMatrices> camera;

struct VOut
{
    float4 position : SV_Position;
    float3 normal;
    float2 uv;
}

[shader("vertex")]
VOut vertexMain(
    VertexData v,
    uniform Params params
)
{
    VOut output;

    float4 worldPos = float4(v.position, 1.0);
    float4 viewPos = mul(worldPos, camera[0].ViewMatrix);
    output.position = mul(viewPos, camera[0].ProjMatrix);
    output.normal = v.normal;
    output.uv = v.uv;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VOut input, uniform Params params) : SV_Target
{
    float4 col = textures[params.DiffuseMapIndex].Sample(input.uv);
    if (col.a < 0.1) discard;
    col.a = 1;
    return col;
}
