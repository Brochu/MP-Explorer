#include <stdio.h>

#include "SDL.h"
//#include "imgui.h"

#define WIDTH 800
#define HEIGHT 600
#define TITLE "[pre-alpha] MP-Explorer"
#define WINDOW_POS SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED

int main (int argc, char **argv) {
    printf("[MAIN] Setting up SDL2 ...\n");
    SDL_Window *window = nullptr;
    SDL_Surface *surface = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[ERR] Failed to init SDL2 : %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow(TITLE, WINDOW_POS, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("[ERR] Failed to create SDL2 window %s\n", SDL_GetError());
        return 2;
    }
    surface = SDL_GetWindowSurface(window);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
    SDL_UpdateWindowSurface(window);

    //printf("[MAIN] Setting up ImGUI lib ...\n");
    //IMGUI_CHECKVERSION();
    //ImGui::CreateContext();

    //TODO: Application logic runs...
    SDL_Event e;
    bool quit = false;
    while(quit == false) {
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
        }
    }

    printf("[MAIN] Teardown SDL2 ...\n");
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
