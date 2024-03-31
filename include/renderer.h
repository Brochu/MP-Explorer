#pragma once

#include <d3d12.h>
#include <DirectXMath.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Renderer {

void Init(HWND hwnd, int width, int height);
void Teardown();

}
