#include "app.h"
#include "renderer.h"

#include "SDL.h"
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace App {
#define WIDTH 1024
#define HEIGHT 768
#define TITLE "[pre-alpha] MP-Explorer"
#define WINDOW_POS SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED

bool running = true;
HWND hwnd;
SDL_Window *window;

void setup() {
    printf("[MAIN] Setting up SDL2 ...\n");

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
}

void step() {
    //TODO: Add some Tracy reporting here, root of frame logic

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT || (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)) {
            running = false;
        }
    }

    Render::frame();
}

void teardown() {
    printf("[MAIN] Teardown SDL2 ...\n");
    SDL_DestroyWindow(window);
    SDL_Quit();
}

}
