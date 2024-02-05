#include "app.h"
#include "debugui.h"
#include "renderer.h"

#include "SDL.h"
#include "Tracy.hpp"

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

UINT64 rootSigIndex = 0;
UINT64 PSOIndex = 0;

Vertex tri[] = {
    { {0.f, 0.25f, 0.f}, {1.f, 0.f} },
    { {0.25f, -0.25f, 0.f}, {0.f, 1.f} },
    { {-0.25f, -0.25f, 0.f}, {1.f, 1.f} },
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
    PSOIndex = Render::CreatePSO();
    UploadData verts[1] { {tri} }; //TODO: Is there a better way to handle this? Try with real data?
    Render::UploadVertexData(verts, draws);
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

    Render::StartFrame();
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
