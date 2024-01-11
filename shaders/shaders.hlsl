struct PSInput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float4 pos : POSITION, float2 tex : TEXCOORD0) {
    PSInput o;
    o.pos = pos;
    o.color = float4(tex, 1.0, 1.0);
    return o;
}

float4 PSMain(PSInput input) : SV_TARGET {
    return input.color;
}
