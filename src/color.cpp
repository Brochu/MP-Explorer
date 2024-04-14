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
    //TODO: Inverse operation from ToSRGB()
    return {};
}
Color ToREC709(Color c) {
    return {};
}
Color FromREC709(Color c) {
    //TODO: Invert operation form ToREC709()
    return {};
}
