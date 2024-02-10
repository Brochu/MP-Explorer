#include "debugui.h"
#include "renderer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_dx12.h"

namespace UI {
using namespace DirectX;

bool showDemo = true;
bool showDebug = true;

void InitApp(SDL_Window *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForD3D(window);
}

void InitRender(ID3D12Device *device, int frameCount, DXGI_FORMAT format, ID3D12DescriptorHeap *heap) {
    ImGui_ImplDX12_Init(device, frameCount, format,
        heap,
        heap->GetCPUDescriptorHandleForHeapStart(),
        heap->GetGPUDescriptorHandleForHeapStart()
    );
}

void Update(SDL_Event *event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}

void DrawUI(Camera &cam) {
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplDX12_NewFrame();
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
}

void EndFrame(ID3D12GraphicsCommandList *cmdlist) {
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdlist);
}

void Teardown() {
    ImGui_ImplSDL2_Shutdown();
    ImGui_ImplDX12_Shutdown();
    ImGui::DestroyContext();
}

}
