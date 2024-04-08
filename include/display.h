#pragma once
#include <cstdint>
#include <d3dx12.h>

namespace Display {

void Init(HWND hwnd);
void Teardown();
void Resize(uint32_t width, uint32_t height);
void Present();

}

namespace Graphics {

extern uint32_t g_displayWidth;
extern uint32_t g_displayHeight;

uint64_t GetFrameCount();
float GetFrameTime();
float GetFrameRate();

}
