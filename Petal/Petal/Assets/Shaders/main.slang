// http://shader-slang.org/slang-playground/
// This example demonstrates how to define vertex and fragment shader entrypoints.
// This is a compile-only example, so there is no output from the playground.
// You can use the "Compile" button to compile this example to different targets.

#include "Common.h"

//StructuredBuffer<Camera> camera;
StructuredBuffer<VertexData> vertices;
StructuredBuffer<uint> indices;
Sampler2D textures[];

struct VOut
{
    float4 position : SV_Position;
    float3 normal;
    float2 uv;
}

[shader("vertex")]
VOut vertexMain(uint vertexID : SV_VertexID)
{
    uint index = indices[vertexID];
    VertexData v = vertices[index];

    VOut output;
    output.position = float4(v.position, 1.0);
    //output.position = mul(camera[0].mvp, float4(v.position, 1.0));
    output.normal = v.normal;
    output.uv = v.uv;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VOut input) : SV_Target
{
    float4 col = textures[1].Sample(input.uv);
    if (col.a < 0.1) discard;
    col.a = 1;
    return col;
}
