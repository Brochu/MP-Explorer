#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d12.h>
#include <DirectXMath.h>

#include <span>
#include <vector>

struct Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 tex;
};

struct UploadData {
    std::span<Vertex> verts;
    std::span<UINT> indices;
};

struct Draws {
    std::vector<UINT> idxCount;
    std::vector<UINT> idxStart;
    std::vector<UINT> vertStart;
};

struct Camera {
    float fov;
    float ratio;
    float nearp;
    float farp;

    DirectX::XMVECTOR pos;
    DirectX::XMVECTOR forward;
    DirectX::XMVECTOR up;

    constexpr static float min_fov = 5.f;
    constexpr static float max_fov = 125.f;
};

struct D3D12_ROOT_PARAMETER;
struct D3D12_STATIC_SAMPLER_DESC;

namespace Render {

void setup(HWND hwnd, int width, int height);
void StartFrame(std::span<D3D12_VIEWPORT> viewports, std::span<D3D12_RECT> scissors, int rootSigIndex, int psoIndex);
void EndFrame();
void teardown();

int CreateRootSignature(std::span<D3D12_ROOT_PARAMETER> params, std::span<D3D12_STATIC_SAMPLER_DESC> samplers);
int CreatePSO(LPCWSTR shaderFile, LPCWSTR vertEntry, LPCWSTR pixEntry);
Draws UploadDrawData(std::span<UploadData> uploadData);

Camera initCamera(int width, int height);
void UseCamera(Camera &cam);
void RecordDraws(UINT idxCount, UINT idxStart, INT vertOffset);

}
