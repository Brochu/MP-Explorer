#pragma once
#include <DirectXMath.h>

struct Color { 
    DirectX::XMVECTORF32 vals;
};

Color MakeColor();
Color MakeColor(DirectX::XMVECTORF32 vec);
Color MakeColor(const DirectX::XMVECTORF32 &vec);
Color MakeColor(float r, float g, float b, float a = 1.f);
Color MakeColor(uint16_t r, uint16_t g, uint16_t b, uint16_t a = 255, uint16_t bitDepth = 8);
