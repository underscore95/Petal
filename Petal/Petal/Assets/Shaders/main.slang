// http://shader-slang.org/slang-playground/
// This example demonstrates how to define vertex and fragment shader entrypoints.
// This is a compile-only example, so there is no output from the playground.
// You can use the "Compile" button to compile this example to different targets.

#include "Common.h"

Sampler2D textures[];
ConstantBuffer<CameraMatrices> camera;

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
    worldPos.xyz *= 0.15;
    float4 viewPos = mul(worldPos, camera.ViewMatrix);
    output.position = mul(viewPos, camera.ProjMatrix);
    output.normal = v.normal;
    output.uv = v.uv;
    return output;
}

struct FOut {
    float4 Color;
}

[shader("fragment")]
FOut fragmentMain(VOut input, uniform Params params) : SV_Target
{
    FOut out;
    out.Color = textures[params.DiffuseMap].Sample(input.uv);

    if (out.Color.a < 0.1) discard;
    out.Color.a = 1;

    return out;
}
