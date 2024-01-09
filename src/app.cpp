#include "app.h"
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <stdio.h>

namespace App {
#define WIDTH 1026
#define HEIGHT 768
#define TITLE "[pre-alpha] MP-Explorer"
#define WINDOW_POS SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED

static bool running = true;
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
}

void step() { //TODO: Input handling && app logic?
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT || (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)) {
            running = false;
        }
    }
}

void teardown() {
    printf("[MAIN] Teardown SDL2 ...\n");
    SDL_DestroyWindow(window);
    SDL_Quit();
}

}
