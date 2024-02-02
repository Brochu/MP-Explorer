#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DirectXMath.h>

#include <span>

struct Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 tex;
};

struct Camera {
    float fov;
    float ratio;
    float nearp;
    float farp;

    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 forward;
    DirectX::XMFLOAT3 up;
};

namespace Render {

void setup(HWND hwnd, int width, int height);
void StartFrame();
void EndFrame();
void teardown();

uint64_t CreateRootSignature();
uint64_t CreatePSO();
void UploadVertexData(std::span<Vertex> upload, uint64_t &startIndex, size_t &drawCount);

void UseCamera(Camera &cam);
void RecordDraws();


}
