#include "app.h"
#include "debugui.h"
#include "renderer.h"

#include "SDL.h"
#include "Tracy.hpp"
#include <d3dx12.h>

#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <filesystem>

namespace App {
#define WIDTH 1024
#define HEIGHT 768
#define TITLE "[pre-alpha] MP-Explorer"
#define WINDOW_POS SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED

bool running = true;
HWND hwnd;
SDL_Window *window;

D3D12_VIEWPORT vp[] = { CD3DX12_VIEWPORT(0.f, 0.f, (float)WIDTH, (float)HEIGHT) };
D3D12_RECT rect[] = { CD3DX12_RECT(0, 0, WIDTH, HEIGHT) };

int rootSigIndex = 0;
int PSOIndex = 0;

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

void cameraInputs(SDL_Event *e);

void setup() {
    printf("[APP] Data path is setup %s\n", PATH);
    for (const auto &entry : std::filesystem::directory_iterator(PATH)) {
        printf(" - '%ls'\n", entry.path().c_str());
    }
    printf("[APP] Setting up SDL2 ...\n");

    window = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[ERR] Failed to init SDL2 : %s\n", SDL_GetError());
    }
    window = SDL_CreateWindow(TITLE, WINDOW_POS, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("[ERR] Failed to create SDL2 window %s\n", SDL_GetError());
    }
    running = true;

    hwnd = GetActiveWindow();
    Render::setup(hwnd, WIDTH, HEIGHT);
    UI::setup(window);

    D3D12_ROOT_PARAMETER camCBV;
    CD3DX12_ROOT_PARAMETER::InitAsConstantBufferView(camCBV, 0);
    rootSigIndex = Render::CreateRootSignature({ &camCBV, 1 }, {});
    PSOIndex = Render::CreatePSO(L"shaders\\shaders.hlsl", L"VSMain", L"PSMain");

    UploadData model[1] { {cube, idx} }; //TODO: Is there a better way to handle this? Try with real data?
    draws = Render::UploadDrawData(model);
    cam = Render::initCamera(WIDTH, HEIGHT);
}

void step() {
    ZoneScoped;

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        cameraInputs(&e);
        UI::update(&e);

        if (e.type == SDL_QUIT || (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)) {
            running = false;
        }
    }

    //TODO: Check if we really need start/end frame functions
    Render::StartFrame(vp, rect, rootSigIndex, PSOIndex);
    Render::UseCamera(cam);
    Render::RecordDraws(draws.idxCount[0], draws.idxStart[0], draws.vertStart[0]);
    UI::drawUI(cam);
    Render::EndFrame();

    FrameMark;
}

void teardown() {
    Render::teardown();

    printf("[APP] Teardown SDL2 ...\n");
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void cameraInputs(SDL_Event *e) {
    if (e->type == SDL_MOUSEWHEEL) {
        if (e->wheel.y < 0) {
            cam.fov += 2;
        }
        else if (e->wheel.y > 0) {
            cam.fov -= 2;
        }
    }
}

}
