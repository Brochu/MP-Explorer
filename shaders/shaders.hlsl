cbuffer PerFrame : register(b0) {
    float4x4 mvp;
    float4 color;
};

struct PSInput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float3 pos : POSITION, float2 tex : TEXCOORD0) {
    PSInput o;
    o.pos = float4(pos, 1.0);
    o.color = color;
    return o;
}

float4 PSMain(PSInput input) : SV_TARGET {
    return input.color;
}
