ByteAddressBuffer buffers[] : register (t0);

struct Vertex {
    float3 position;
    float2 uv;
    float3 normal;
};
uint VertexBufferIdx = 0;

cbuffer PerFrame : register(b0) {
    float4x4 mvp;
};

PSInput VSMain(float3 pos : POSITION, float2 tex : TEXCOORD0) {
    PSInput o;
    o.pos = mul(float4(pos, 1.0), mvp);
    o.color = float4(tex.x, tex.y, 0.0, 1.0);
    return o;
}

struct PSInput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 PSMain(PSInput input) : SV_TARGET {
    return input.color;
}
