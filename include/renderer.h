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

struct D3D12_ROOT_PARAMETER;
struct D3D12_STATIC_SAMPLER_DESC;

namespace Render {

void setup(HWND hwnd, int width, int height);
void initImGui();
void StartFrame();
void EndFrame();
void teardown();

UINT64 CreateRootSignature(std::span<D3D12_ROOT_PARAMETER> params, std::span<D3D12_STATIC_SAMPLER_DESC> samplers);
UINT64 CreatePSO();
void UploadVertexData(std::span<UploadData> uploadData, Draws &draws);

void UseCamera(Camera &cam);
void RecordDraws(UINT64 rootSigIndex, UINT64 psoIndex);


}
