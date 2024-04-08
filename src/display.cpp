#include "display.h"

namespace Display {

void Init() {
}
void Teardown() {
}
void Resize(uint32_t width, uint32_t height) {
}
void Present() {
}

}

namespace Graphics {

uint32_t g_displayWidth = 800;
uint32_t g_displayHeight = 600;

uint64_t GetFrameCount() {
    return 0;
}

float GetFrameTime() {
    return 0.f;
}

float GetFrameRate() {
    return 0.f;
}

}
