#include "camera.h"

#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_mouse.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

namespace App {
using namespace DirectX;

XMVECTOR basePos = { -5.f, -5.f, -5.f };
XMVECTOR baseFwd = { 1.f, 1.f, 1.f };
XMVECTOR baseUp = { 0.f, 1.f, 0.f };

Camera initCamera(float width, float height) {
    return { 45.f, (float)width / height, 0.1f, 100000.f, basePos, baseFwd, baseUp };
}

void updateCamera(SDL_Event *e, CameraInputs &inputs, Camera &cam) {
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_w) inputs.fwd = true;
        if (e->key.keysym.sym == SDLK_s) inputs.bwd = true;
        if (e->key.keysym.sym == SDLK_a) inputs.left = true;
        if (e->key.keysym.sym == SDLK_d) inputs.right = true;
        if (e->key.keysym.sym == SDLK_q) inputs.up = true;
        if (e->key.keysym.sym == SDLK_e) inputs.down = true;
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

    if (e->type == SDL_MOUSEMOTION) {
        //TODO: Handle updating camera angles with this
        //TODO: Look into setting to invert mouse movements
        int dx = e->motion.xrel;
        int dy = e->motion.yrel;

        inputs.theta += (dx * Camera::hsens);
        if (inputs.theta > 359.f) inputs.theta -= 359;
        if (inputs.theta < 0.f) inputs.theta += 359;
        inputs.phi = min(max(inputs.phi - (dy * Camera::vsens), -89.f), 89.f);

        printf("[CAM] relative mouse motion (%i, %i)\n", dx, dy);
        printf("rotation : theta: %f, phi: %f\n", inputs.theta, inputs.phi);
    }
}

void moveCamera(Camera &cam, CameraInputs &inputs, float delta, float elapsed) {
    //TODO: Rotate camera forward vector
    // Rotate the base forward vector with these angles
    // Use rotated forward vector for cam movement
    //const XMVECTOR quat = XMQuaternionRotationRollPitchYaw(inputs.phi, inputs.theta, 0.f);

    // Move camera position
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
