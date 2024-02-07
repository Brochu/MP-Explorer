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
int vbufferIndex = 0;

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

    rootSigIndex = Render::CreateRootSignature({}, {});
    PSOIndex = Render::CreatePSO(L"shaders\\shaders.hlsl", L"VSMain", L"PSMain");
    UploadData model[1] { {cube, idx} }; //TODO: Is there a better way to handle this? Try with real data?
    vbufferIndex = Render::UploadDrawData(model, draws);

    for (int i = 0; i < draws.vertStart.size(); i++) {
        printf("[APP] draw[%i] -> (%i, %i)\n", i, draws.vertStart[i], draws.vertCount[i]);
    }
}

void step() {
    ZoneScoped;

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        UI::update(&e);

        if(e.type == SDL_QUIT || (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)) {
            running = false;
        }
    }

    //TODO: Check if we really need start/end frame functions
    Render::StartFrame(vp, rect);
    Render::RecordDraws(rootSigIndex, PSOIndex, vbufferIndex, draws.vertStart[0], draws.vertCount[0]);
    UI::drawUI();
    Render::EndFrame();

    FrameMark;
}

void teardown() {
    Render::teardown();

    printf("[APP] Teardown SDL2 ...\n");
    SDL_DestroyWindow(window);
    SDL_Quit();
}

}
