#pragma once
struct SDL_Window;
union SDL_Event;

namespace UI {

void setup(SDL_Window *window);
void update(SDL_Event *event);
void drawUI();
void teardown();

}
