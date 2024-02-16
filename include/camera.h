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

struct CameraInputs {
    bool fwd, bwd;
    bool left, right;
    bool up, down;
};

union SDL_Event;

namespace App {

Camera initCamera(float width, float height);
void updateCamera(SDL_Event *e, CameraInputs &inputs, Camera &cam);
void moveCamera(Camera &cam, CameraInputs inputs, float delta, float elapsed);

};
