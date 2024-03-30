#include "app.h"
#include "camera.h"
#include "debugui.h"
#include "renderer.h"
#include "world.h"

#include "SDL.h"
#include "Tracy.hpp"

#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DirectXMath.h>

struct CamMatrices {
    DirectX::XMMATRIX mvp;
};

namespace App {
using namespace DirectX;

#define WIDTH 1024
#define HEIGHT 768
#define TITLE "[pre-alpha] MP-Explorer"
#define WINDOW_POS SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED

World world;

HWND hwnd;
SDL_Window *window;

UINT ssOriginX = 0;
UINT ssOriginY = 0;

size_t PSOIndex = 0;
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
size_t vertBufIdx;
size_t indexBufIdx;

Camera cam;
CameraInputs camIn;
CamMatrices camMat;

void setup();

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

void setup() {
    printf("[APP] Loading world config data ...\n");
    world = Config::initWorld();

    printf("[APP] Setting up SDL2 ...\n");
    window = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[ERR] Failed to init SDL2 : %s\n", SDL_GetError());
    }
    window = SDL_CreateWindow(TITLE, WINDOW_POS, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("[ERR] Failed to create SDL2 window %s\n", SDL_GetError());
    }
    SDL_SetRelativeMouseMode(SDL_TRUE);

    hwnd = GetActiveWindow();
    UI::initApp(window);

    printf("[APP] Init renderer ...\n");
    Render::Setup(hwnd, WIDTH, HEIGHT);
    RootSigParam params[] { {RootSigParam::Type::CBVDescriptor, 0} };
    Render::CreateRootSignature(params, {});
    PSOIndex = Render::CreatePSO(L"shaders/shaders.hlsl");
    vertBufIdx = Render::UploadVertBuffer(nullptr, 0l, 0l);
    indexBufIdx = Render::UploadIndexBuffer(nullptr, 0l, 0l);
}

bool update(float delta, float elapsed) {
    ZoneScoped;

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        UI::update(&e);

        if (e.type == SDL_QUIT || (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)) {
            return false;
        }
        updateCamera(&e, camIn, cam, delta, elapsed);
    }
    moveCamera(cam, camIn, delta, elapsed);

    return true;
}

void render() {
    ZoneScoped;
}

}
