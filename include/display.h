#pragma once
#include <cstdint>

namespace Display {

void Init();
void Teardown();
void Resize(uint32_t width, uint32_t height);
void Present();

}

namespace Graphics {
}
