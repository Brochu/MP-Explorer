#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DirectXMath.h>

namespace Render {

using namespace DirectX;
struct Vertex {
    XMFLOAT3 pos;
    XMFLOAT2 tex;
};

void setup(HWND hwnd, int width, int height);
void StartFrame();
void EndFrame();
void teardown();

void UploadData();
void RecordDraws();

}
