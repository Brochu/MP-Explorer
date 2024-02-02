#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DirectXMath.h>

#include <span>
#include <vector>

struct Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 tex;
};

struct UploadData {
    std::span<Vertex> verts;
};

struct Draws {
    std::vector<UINT64> startIndex;
    std::vector<UINT64> vertCount;
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

UINT64 CreateRootSignature();
UINT64 CreatePSO();
void UploadVertexData(std::span<UploadData> uploadData, Draws &draws);

void UseCamera(Camera &cam);
void RecordDraws();


}
