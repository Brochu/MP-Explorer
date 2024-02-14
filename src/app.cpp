#include "app.h"
#include "debugui.h"
#include "renderer.h"

#include "SDL.h"
#include "Tracy.hpp"

#include <d3dx12.h>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct CamMatrices {
    DirectX::XMMATRIX mvp;
};

namespace App {
using namespace DirectX;

#define WIDTH 1024
#define HEIGHT 768
#define TITLE "[pre-alpha] MP-Explorer"
#define WINDOW_POS SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED

HWND hwnd;
SDL_Window *window;

D3D12_VIEWPORT vp[] = { CD3DX12_VIEWPORT(0.f, 0.f, (float)WIDTH, (float)HEIGHT) };
D3D12_RECT rect[] = { CD3DX12_RECT(0, 0, WIDTH, HEIGHT) };

int rootSigIndex = 0;
int PSOIndex = 0;
int camCBIndex = 0;

Vertex cube[] = {
    { {-0.25f, -0.25f, -0.25f}, {0.f, 0.f} },
    { { 0.25f, -0.25f, -0.25f}, {1.f, 0.f} },
    { { 0.25f,  0.25f, -0.25f}, {1.f, 1.f} },
    { {-0.25f,  0.25f, -0.25f}, {0.f, 1.f} },
    { {-0.25f, -0.25f,  0.25f}, {0.f, 0.f} },
    { { 0.25f, -0.25f,  0.25f}, {1.f, 0.f} },
    { { 0.25f,  0.25f,  0.25f}, {1.f, 1.f} },
    { {-0.25f,  0.25f,  0.25f}, {0.f, 1.f} },
};
UINT idx[] = {
    0, 1, 3, 3, 1, 2,
    1, 5, 2, 2, 5, 6,
    5, 4, 6, 6, 4, 7,
    4, 0, 7, 7, 0, 3,
    3, 2, 7, 7, 2, 6,
    4, 5, 0, 0, 5, 1,
};
Draws draws;

Camera cam;
struct CamInputs {
    bool fwd, bwd;
    bool left, right;
    bool up, down;
} camInputs;
CamMatrices matrices;

void setup();
void step();

bool update(float delta, float elapsed);
void render();

int run() {
    UINT64 startStamp;
    UINT64 lastStamp;
    startStamp = lastStamp = SDL_GetTicks64();
    setup();

    bool running = true;
    while (running) {
        UINT64 current = SDL_GetTicks64();
        float delta = (current - lastStamp) / 1000.f;
        float elapsed = (current - startStamp) / 1000.f;
        lastStamp = current;

        running = update(delta, elapsed);
        render();
    }

    Render::Teardown();
    UI::teardown();

    printf("[APP] Teardown SDL2 ...\n");
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

Camera initCamera(float width, float height);
void updateCamera(float delta, float elapsed);

void setup() {
    printf("[APP] Setting up SDL2 ...\n");
    window = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[ERR] Failed to init SDL2 : %s\n", SDL_GetError());
    }
    window = SDL_CreateWindow(TITLE, WINDOW_POS, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("[ERR] Failed to create SDL2 window %s\n", SDL_GetError());
    }

    hwnd = GetActiveWindow();
    UI::initApp(window);
    Render::Setup(hwnd, WIDTH, HEIGHT);

    //TODO: Find a simpler structure to specify root signature parameters
    D3D12_ROOT_PARAMETER camCBV;
    CD3DX12_ROOT_PARAMETER::InitAsConstantBufferView(camCBV, 0);
    rootSigIndex = Render::CreateRootSignature({ &camCBV, 1 }, {});
    PSOIndex = Render::CreatePSO(L"shaders\\shaders.hlsl", L"VSMain", L"PSMain");

    UploadData model[1] { {cube, idx} }; //TODO: Is there a better way to handle this? Try with real data?
    draws = Render::UploadDrawData(model);
    cam = initCamera(WIDTH, HEIGHT);
    camCBIndex = Render::CreateBufferedCB(sizeof(CamMatrices));
}

bool update(float delta, float elapsed) {
    ZoneScoped;

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        UI::update(&e);

        if (e.type == SDL_QUIT || (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)) {
            return false;
        }

        if (e.type == SDL_KEYDOWN) {
            camInputs.fwd = e.key.keysym.sym == SDLK_w;
            camInputs.bwd = e.key.keysym.sym == SDLK_s;
            camInputs.left = e.key.keysym.sym == SDLK_a;
            camInputs.right = e.key.keysym.sym == SDLK_d;
            camInputs.up = e.key.keysym.sym == SDLK_q;
            camInputs.down = e.key.keysym.sym == SDLK_e;
        }
        else if (e.type == SDL_KEYUP) {
            if (e.key.keysym.sym == SDLK_w) camInputs.fwd = false;
            if (e.key.keysym.sym == SDLK_s) camInputs.bwd = false;
            if (e.key.keysym.sym == SDLK_a) camInputs.left = false;
            if (e.key.keysym.sym == SDLK_d) camInputs.right = false;
            if (e.key.keysym.sym == SDLK_q) camInputs.up = false;
            if (e.key.keysym.sym == SDLK_e) camInputs.down = false;
        }

        if (e.type == SDL_MOUSEWHEEL) {
            if (e.wheel.y > 0) {
                cam.fov = max(Camera::min_fov, cam.fov - Camera::fov_speed);
            }
            else if (e.wheel.y < 0) {
                cam.fov = min(Camera::max_fov, cam.fov + Camera::fov_speed);
            }
        }
    }

    updateCamera(delta, elapsed);
    return true;
}

void render() {
    ZoneScoped;

    //TODO: I have to review this process, interface is bad
    Render::StartFrame(vp, rect, rootSigIndex, PSOIndex);
    Render::BindBufferedCB(camCBIndex, (void*)&matrices, sizeof(CamMatrices));
    Render::RecordDraws(draws.idxCount[0], draws.idxStart[0], draws.vertStart[0]);
    UI::drawUI(cam);
    Render::EndFrame();
}

Camera initCamera(float width, float height) {
    return {
        45.f, (float)width / height, 0.1f, 100000.f,
        {-5.f, -5.f, -5.f}, {1.f, 1.f, 1.f}, {0.f, 1.f, 0.f}
    };
}

void updateCamera(float delta, float elapsed) {
    if (camInputs.fwd) {
        cam.pos += XMVector3Normalize(cam.forward) * delta * Camera::speed;
    }
    else if (camInputs.bwd) {
        cam.pos -= XMVector3Normalize(cam.forward) * delta * Camera::speed;
    }

    if (camInputs.left) {
        XMVECTOR left = XMVector3Normalize(XMVector3Cross(cam.forward, cam.up));
        cam.pos += left * delta * Camera::speed;
    }
    else if (camInputs.right) {
        XMVECTOR left = XMVector3Normalize(XMVector3Cross(cam.forward, cam.up));
        cam.pos -= left * delta * Camera::speed;
    }

    if (camInputs.up) {
        cam.pos += XMVector3Normalize(cam.up) * delta * Camera::speed;
    }
    else if (camInputs.down) {
        cam.pos -= XMVector3Normalize(cam.up) * delta * Camera::speed;
    }

    XMMATRIX model = XMMatrixIdentity();
    XMMATRIX view = XMMatrixLookToLH(cam.pos, cam.forward, cam.up);
    XMMATRIX persp = XMMatrixPerspectiveFovLH(XMConvertToRadians(cam.fov), cam.ratio, cam.nearp, cam.farp);
    matrices.mvp = model * view * persp;
}

}
