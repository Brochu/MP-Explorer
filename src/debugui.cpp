#include "debugui.h"
#include "renderer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"

namespace UI {
using namespace DirectX;

bool showDemo = true;
bool showDebug = true;

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
    }
    if (showDebug) {
        ImGui::Begin("Debug", &showDebug);

        ImGui::BeginGroup();
        ImGui::Text("Camera values:");
        ImGui::Text("FoV: %f", cam.fov);
        ImGui::Text("Aspect Ratio: %f", cam.ratio);
        ImGui::Text("Near Plane: %f", cam.nearp);
        ImGui::Text("Far Plane: %f", cam.farp);

        XMFLOAT3 vec;
        XMStoreFloat3(&vec, cam.pos);
        ImGui::Text("Position: (%f, %f, %f)", vec.x, vec.y, vec.z);
        XMStoreFloat3(&vec, cam.forward);
        ImGui::Text("Forward: (%f, %f, %f)", vec.x, vec.y, vec.z);
        XMStoreFloat3(&vec, cam.up);
        ImGui::Text("Up: (%f, %f, %f)", vec.x, vec.y, vec.z);
        ImGui::EndGroup();

        ImGui::End();
    }
    ImGui::Render();
}

void teardown() {
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

}
