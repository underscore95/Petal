// http://shader-slang.org/slang-playground/
// This example demonstrates how to define vertex and fragment shader entrypoints.
// This is a compile-only example, so there is no output from the playground.
// You can use the "Compile" button to compile this example to different targets.

struct VertexData
{
    float3 position;
    float3 normal;
    float2 uv;
};

struct Camera
{
    float4x4 mvp;
}

StructuredBuffer<Camera> camera;
StructuredBuffer<VertexData> vertices;

struct VOut
{
    float4 position : SV_Position;
    float3 normal;
    float2 uv;
}

[shader("vertex")]
VOut vertexMain(uint vertexID : SV_VertexID)
{
    VertexData v = vertices[vertexID];

    VOut output;
    output.position = mul(camera[0].mvp, float4(v.position, 1.0));
    output.normal = v.normal;
    output.uv = v.uv;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VOut input) : SV_Target
{
    return float4(1,1,1,1);
}
