#pragma once
#include "d3d12.h"

struct Camera;

struct SDL_Window;
union SDL_Event;

struct ID3D12Device;

namespace UI {

void initApp(SDL_Window *window);
void initRender(ID3D12Device *device, int frameCount, DXGI_FORMAT format, ID3D12DescriptorHeap *heap);
void update(SDL_Event *event);
void drawUI(Camera &cam);
void endFrame(ID3D12GraphicsCommandList *cmdlist);
void teardown();

}
