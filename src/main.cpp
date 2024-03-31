#include "renderer.h"
#include "world.h"

#include "SDL.h"

#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DirectXMath.h>

#define WIDTH 1024
#define HEIGHT 768
#define TITLE "[pre-alpha] MP-Explorer"
#define WINDOW_POS SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED

enum Code {
    OK,
    SDL_ERR,
    APP_CLOSE,
};

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

UINT64 startStamp;
UINT64 lastStamp;

HWND hwnd;
SDL_Window *window;

Code Init() {
    startStamp = lastStamp = SDL_GetTicks64();

    printf("[APP] Setting up SDL2 ...\n");
    window = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[ERR] Failed to init SDL2 : %s\n", SDL_GetError());
        return Code::SDL_ERR;
    }
    window = SDL_CreateWindow(TITLE, WINDOW_POS, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("[ERR] Failed to create SDL2 window %s\n", SDL_GetError());
        return Code::SDL_ERR;
    }
    hwnd = GetActiveWindow();
    SDL_SetRelativeMouseMode(SDL_TRUE);

    printf("[APP] Init renderer ...\n");
    Render::Init(hwnd, WIDTH, HEIGHT);
    return Code::OK;
}

Code Update(float delta, float elapsed) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {

        if (e.type == SDL_QUIT || (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)) {
            return Code::APP_CLOSE;
        }
    }

    return Code::OK;
}

Code Draw() {
    return Code::OK;
}

int main(int argc, char **argv) {
    //TODO: Parse command line args

    int initRet = Init();
    if (initRet != Code::OK) {
        printf("[MAIN] Could not complete setup, error code = %i\n", initRet);
        return -1;
    }

    bool running = true;
    while (running) {
        UINT64 current = SDL_GetTicks64();
        float delta = (current - lastStamp) / 1000.f;
        float elapsed = (current - startStamp) / 1000.f;
        lastStamp = current;

        int upRet = Update(delta, elapsed);
        if (upRet == Code::APP_CLOSE) {
            running = false;
        }
        else if (upRet != Code::OK) {
            printf("[MAIN] Could not complete update sequence, error code = %i\n", upRet);
        }

        int renderRet = Draw();
        if (renderRet != 0) {
            printf("[MAIN] Could not complete setup, error code = %i\n", renderRet);
        }
    }

    Render::Teardown();

    printf("[APP] Teardown SDL2 ...\n");
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
