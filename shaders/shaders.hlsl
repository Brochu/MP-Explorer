#define VBUFFERIDX 0
ByteAddressBuffer buffers[] : register (t0);

struct Vertex {
    float3 position;
    float2 uv;
    float3 normal;
};

cbuffer PerFrame : register(b0) {
    float4x4 mvp;
};

struct PSInput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(uint vertId : SV_VertexID) {
    PSInput o;
    o.pos = float4(1.0, 1.0, 1.0, 1.0); //mul(float4(pos, 1.0), mvp);
    o.color = float4(1.0, 1.0, 1.0, 1.0); //float4(tex.x, tex.y, 0.0, 1.0);
    return o;
}

float4 PSMain(PSInput input) : SV_TARGET {
    return input.color;
}
