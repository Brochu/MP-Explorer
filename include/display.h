#pragma once
#include <cstdint>

namespace Display {

void Init();
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
