#pragma once
#include <DirectXMath.h>

struct Color { 
    DirectX::XMVECTORF32 vals;

    float R() const { return DirectX::XMVectorGetX(vals); }
    float G() const { return DirectX::XMVectorGetY(vals); }
    float B() const { return DirectX::XMVectorGetZ(vals); }
    float A() const { return DirectX::XMVectorGetW(vals); }

    void SetR(float r) { vals.f[0] = r; }
    void SetG(float g) { vals.f[1] = g; }
    void SetB(float b) { vals.f[2] = b; }
    void SetA(float a) { vals.f[3] = a; }
    void SetRGB(float r, float g, float b) {
        vals.v = DirectX::XMVectorSelect(vals, DirectX::XMVectorSet(r, g, b, 0.f), DirectX::g_XMMask3);
    };

    float *GetPtr() { return vals.f; }
    float &operator[](int idx) { return GetPtr()[idx]; }

    bool operator==(const Color &other) { return DirectX::XMVector4Equal(vals, other.vals); }
    bool operator!=(const Color &other) { return DirectX::XMVector4NotEqual(vals, other.vals); }
};

Color MakeColor();
Color MakeColor(DirectX::XMVECTOR vec);
Color MakeColor(const DirectX::XMVECTORF32 &vec);
Color MakeColor(float r, float g, float b, float a = 1.f);
Color MakeColor(uint16_t r, uint16_t g, uint16_t b, uint16_t a = 255, uint16_t bitDepth = 8);

Color ToSRGB(Color c);
Color FromSRGB(Color c);
Color ToREC709(Color c);
Color FromREC709(Color c);

uint32_t R10G10B10A2(Color c);
uint32_t R8G8B8A8(Color c);

inline Color Max(Color a, Color b) { return MakeColor(DirectX::XMVectorMax(a.vals, b.vals)); }
inline Color Min(Color a, Color b) { return MakeColor(DirectX::XMVectorMin(a.vals, b.vals)); }
inline Color Clamp(Color c, Color a, Color b) { return MakeColor(DirectX::XMVectorClamp(c.vals, a.vals, b.vals)); }
