#include "Common.h"

Sampler2D gBufferAlbedo;
Sampler2D gBufferPosition;
Sampler2D gBufferNormal;

struct VOut
{
    float4 Position : SV_Position;
    float2 uv;
}

[shader("vertex")]
VOut vertexMain(
    VertexData v,
    uniform Params params
)
{
    VOut output;
    output.Position = float4(v.position, 1);
    output.uv = v.uv;
    
    return output;
}

struct FOut {
    float4 Albedo;
}

[shader("fragment")]
FOut fragmentMain(VOut input, uniform Params params) : SV_Target
{
    FOut out;
    out.Albedo = gBufferAlbedo.Sample(input.uv);

    return out;
}
