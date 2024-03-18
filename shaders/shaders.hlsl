cbuffer PerFrame : register(b0) {
    float4x4 mvp;
};

struct Vertex {
    float3 position;
    float2 uv;
    float3 normal;
};
StructuredBuffer<Vertex> vertices : register (t0);

struct PSInput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float3 pos : POSITION, float2 tex : TEXCOORD0) {
    PSInput o;
    o.pos = mul(float4(pos, 1.0), mvp);
    o.color = float4(tex.x, tex.y, 0.0, 1.0);
    return o;
}

float4 PSMain(PSInput input) : SV_TARGET {
    return input.color;
}
