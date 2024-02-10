#pragma once
#include "d3d12.h"

struct Camera;

struct SDL_Window;
union SDL_Event;

struct ID3D12Device;

namespace UI {

void InitApp(SDL_Window *window);
void InitRender(ID3D12Device *device, int frameCount, DXGI_FORMAT format, ID3D12DescriptorHeap *heap);
void Update(SDL_Event *event);
void DrawUI(Camera &cam);
void EndFrame(ID3D12GraphicsCommandList *cmdlist);
void Teardown();

}
