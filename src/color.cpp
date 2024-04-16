#include "color.h"

using namespace DirectX;

Color MakeColor(){
    return { g_XMZero };
}
Color MakeColor(DirectX::XMVECTOR vec){
    Color c;
    c.vals.v = vec;
    return c;
}
Color MakeColor(const DirectX::XMVECTORF32 &vec){
    return { vec };
}
Color MakeColor(float r, float g, float b, float a){
    Color c;
    c.vals.v = XMVectorSet(r, g, b, a);
    return c;
}
Color MakeColor(uint16_t r, uint16_t g, uint16_t b, uint16_t a, uint16_t bitDepth){
    Color c;
    c.vals.v = XMVectorSet(r, g, b, a);
    c.vals.v = XMVectorScale(c.vals.v, 1.f / ((1 << bitDepth) - 1));
    return c;
}

Color ToSRGB(Color c) {
    XMVECTOR T = XMVectorSaturate(c.vals);
    XMVECTOR res = XMVectorPow(T, XMVectorReplicate(1.f / 2.4f));
    res = XMVectorScale(res, 1.055f);
    res = XMVectorSubtract(res, XMVectorReplicate(0.055f));

    res = XMVectorSelect(res, XMVectorScale(T, 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
    return MakeColor(XMVectorSelect(T, res, g_XMSelect1110));
}
Color FromSRGB(Color c) {
    XMVECTOR T = XMVectorSaturate(c.vals);
    XMVECTOR res = XMVectorAdd(T, XMVectorReplicate(0.055f));
    res = XMVectorScale(res, 1.0f / 1.055f);
    res = XMVectorPow(res, XMVectorReplicate(2.4f));

    res = XMVectorSelect(res, XMVectorScale(T, 1.0f / 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
    return MakeColor(XMVectorSelect(T, res, g_XMSelect1110));
}
Color ToREC709(Color c) {
    XMVECTOR T = XMVectorSaturate(c.vals);
    XMVECTOR res = XMVectorPow(T, XMVectorReplicate(0.45f));
    res = XMVectorScale(res, 1.099f);
    res = XMVectorSubtract(res, XMVectorReplicate(0.099f));

    res = XMVectorSelect(res, XMVectorScale(T, 4.5f), XMVectorLess(T, XMVectorReplicate(0.0018f)));
    return MakeColor(XMVectorSelect(T, res, g_XMSelect1110));

}
Color FromREC709(Color c) {
    XMVECTOR T = XMVectorSaturate(c.vals);
    XMVECTOR res = XMVectorAdd(T, XMVectorReplicate(0.099f));
    res = XMVectorScale(res, 1.0f / 1.099f);
    res = XMVectorPow(res, XMVectorReplicate(1.0f / 0.45f));

    res = XMVectorSelect(res, XMVectorScale(T, 1.0f / 4.5f), XMVectorLess(T, XMVectorReplicate(0.0081f)));
    return MakeColor(XMVectorSelect(T, res, g_XMSelect1110));
}

uint32_t R10G10B10A2(Color c) {
    XMVECTOR res = XMVectorMultiply(XMVectorSaturate(c.vals), XMVectorSet(1023.f, 1023.f, 1023.f, 3.f));
    res = XMVectorRound(res);

    res = _mm_castsi128_ps(_mm_cvttps_epi32(res));
    uint32_t r = XMVectorGetIntX(res);
    uint32_t g = XMVectorGetIntY(res);
    uint32_t b = XMVectorGetIntZ(res);
    uint32_t a = XMVectorGetIntW(res) >> 8;
    return a << 30 | b << 20 | g << 10 | r;
}

uint32_t R8G8B8A8(Color c) {
    XMVECTOR res = XMVectorMultiply(XMVectorSaturate(c.vals), XMVectorReplicate(255.f));
    res = XMVectorRound(res);

    res = _mm_castsi128_ps(_mm_cvttps_epi32(res));
    uint32_t r = XMVectorGetIntX(res);
    uint32_t g = XMVectorGetIntY(res);
    uint32_t b = XMVectorGetIntZ(res);
    uint32_t a = XMVectorGetIntW(res);
    return a << 24 | b << 16 | g << 8 | r;
}
