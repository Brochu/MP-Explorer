#include "color.h"

using namespace DirectX;

Color MakeColor(){
    return { g_XMZero };
}
Color MakeColor(DirectX::XMVECTORF32 vec){
    return { vec };
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
