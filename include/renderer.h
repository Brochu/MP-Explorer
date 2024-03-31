#pragma once

#include <d3d12.h>

namespace Renderer {
extern ID3D12Device *g_device;

void Init(HWND hwnd, int width, int height);
void Teardown();

}
