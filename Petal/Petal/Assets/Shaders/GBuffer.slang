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
    float4 Position;
    float4 Normal;
    float4 Albedo;
}

[shader("fragment")]
FOut fragmentMain(VOut input, uniform Params params) : SV_Target
{
    FOut out;
    out.Albedo = textures[params.DiffuseMap].Sample(input.uv); // todo: output UV and image to sample instead?

    if (out.Albedo.a < 0.1) discard;
    out.Albedo.a = 1;

    out.Normal =  float4(input.normal, 0);
    out.Position = input.position;

    return out;
}
