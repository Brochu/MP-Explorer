struct PSInput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float3 pos : POSITION, float2 tex : TEXCOORD0) {
    PSInput o;
    o.pos = float4(pos, 1.0);
    o.color = float4(tex, 0.0, 1.0);
    return o;
}

float4 PSMain(PSInput input) : SV_TARGET {
    return input.color;
}
