#include "debugui.h"
#include "renderer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"

namespace UI {

bool showDemo = true;

void setup(SDL_Window *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForD3D(window);
    Render::initImGui();
}

void update(SDL_Event *event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}

void drawUI(Camera &cam) {
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (showDemo) {
        ImGui::ShowDemoWindow(&showDemo);
        //TODO: More debug controls here
        // Camera values
    }
    ImGui::Render();
}

void teardown() {
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

}
