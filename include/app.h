#pragma once
#include <DirectXMath.h>

struct Camera {
    float fov;
    float ratio;
    float nearp;
    float farp;

    DirectX::XMVECTOR pos;
    DirectX::XMVECTOR forward;
    DirectX::XMVECTOR up;

    constexpr static float min_fov = 5.f;
    constexpr static float max_fov = 125.f;
    constexpr static float speed = 10.f;
    constexpr static float fov_speed = 2.f;
};

namespace App {

int run();

}
