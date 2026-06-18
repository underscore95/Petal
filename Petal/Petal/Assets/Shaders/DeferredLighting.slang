#include "Common.h"

Sampler2D gBufferAlbedo;
Sampler2D gBufferPosition;
Sampler2D gBufferNormal;

ConstantBuffer<DeferredLighting> Lighting;

struct VOut
{
    float4 Position : SV_Position;
    float2 uv;
}

[shader("vertex")]
VOut vertexMain(
    VertexData v,
    uniform Params params)
{
    VOut output;
    output.Position = float4(v.position, 1);
    output.uv = v.uv;

    return output;
}

struct FOut
{
    float4 Albedo;
}

static constexpr float3 AMBIENT = float3(0.1, 0.1, 0.1);

[shader("fragment")]
FOut fragmentMain(VOut input, uniform Params params) : SV_Target
{
    float3 fragPos = gBufferPosition.Sample(input.uv).xyz;

    if (all(fragPos == 0))
        discard;

    float3 normal = normalize(gBufferNormal.Sample(input.uv).xyz * 2.0 - 1.0);

    float4 albedo = gBufferAlbedo.Sample(input.uv);

    float3 lighting = albedo.rgb * AMBIENT;

    for (uint i = 0; i < Lighting.NumLights; i++)
    {
        float3 lightDir = normalize(Lighting.Lights[i].Position - fragPos);

        float strength = max(dot(normal, lightDir), 0.0);

        float3 diffuse = strength * albedo.rgb * Lighting.Lights[i].Color;

        lighting += diffuse;
    }

    FOut out;
    out.Albedo = float4(saturate(lighting), albedo.a);

    return out;
}
