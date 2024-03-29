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
    constexpr static float hsens = 10.0f;
    constexpr static float vsens = 10.0f;
};

struct CameraInputs {
    bool fwd, bwd;
    bool left, right;
    bool up, down;

    float theta; // 0 : 359
    float phi; // -90 : 90
};

union SDL_Event;

namespace App {

Camera initCamera(float width, float height);
DirectX::XMMATRIX &getCameraMVP(Camera &cam);

void updateCamera(SDL_Event *e, CameraInputs &inputs, Camera &cam, float delta, float elapsed);
void moveCamera(Camera &cam, CameraInputs &inputs, float delta, float elapsed);

};
