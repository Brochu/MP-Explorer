#include "camera.h"

#include "SDL_events.h"
#include "SDL_keycode.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace App {
using namespace DirectX;

Camera initCamera(float width, float height) {
    return {
        45.f, (float)width / height, 0.1f, 100000.f,
        {-5.f, -5.f, -5.f}, {1.f, 1.f, 1.f}, {0.f, 1.f, 0.f}
    };
}

void updateCamera(SDL_Event *e, CameraInputs &inputs, Camera &cam) {
    if (e->type == SDL_KEYDOWN) {
        inputs.fwd = e->key.keysym.sym == SDLK_w;
        inputs.bwd = e->key.keysym.sym == SDLK_s;
        inputs.left = e->key.keysym.sym == SDLK_a;
        inputs.right = e->key.keysym.sym == SDLK_d;
        inputs.up = e->key.keysym.sym == SDLK_q;
        inputs.down = e->key.keysym.sym == SDLK_e;
    }
    else if (e->type == SDL_KEYUP) {
        if (e->key.keysym.sym == SDLK_w) inputs.fwd = false;
        if (e->key.keysym.sym == SDLK_s) inputs.bwd = false;
        if (e->key.keysym.sym == SDLK_a) inputs.left = false;
        if (e->key.keysym.sym == SDLK_d) inputs.right = false;
        if (e->key.keysym.sym == SDLK_q) inputs.up = false;
        if (e->key.keysym.sym == SDLK_e) inputs.down = false;
    }

    if (e->type == SDL_MOUSEWHEEL) {
        if (e->wheel.y > 0) {
            cam.fov = max(Camera::min_fov, cam.fov - Camera::fov_speed);
        }
        else if (e->wheel.y < 0) {
            cam.fov = min(Camera::max_fov, cam.fov + Camera::fov_speed);
        }
    }
}

void moveCamera(Camera &cam, CameraInputs inputs, float delta, float elapsed) {
    if (inputs.fwd) {
        cam.pos += XMVector3Normalize(cam.forward) * delta * Camera::speed;
    }
    else if (inputs.bwd) {
        cam.pos -= XMVector3Normalize(cam.forward) * delta * Camera::speed;
    }

    if (inputs.left) {
        XMVECTOR left = XMVector3Normalize(XMVector3Cross(cam.forward, cam.up));
        cam.pos += left * delta * Camera::speed;
    }
    else if (inputs.right) {
        XMVECTOR left = XMVector3Normalize(XMVector3Cross(cam.forward, cam.up));
        cam.pos -= left * delta * Camera::speed;
    }

    if (inputs.up) {
        cam.pos += XMVector3Normalize(cam.up) * delta * Camera::speed;
    }
    else if (inputs.down) {
        cam.pos -= XMVector3Normalize(cam.up) * delta * Camera::speed;
    }
}

};
