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
    std::vector<UINT> vertStart;
    std::vector<UINT> vertCount;
    std::vector<UINT> idxStart;
    std::vector<UINT> idxCount;
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
void StartFrame(std::span<D3D12_VIEWPORT> viewports, std::span<D3D12_RECT> scissors);
void EndFrame();
void teardown();

int CreateRootSignature(std::span<D3D12_ROOT_PARAMETER> params, std::span<D3D12_STATIC_SAMPLER_DESC> samplers);
int CreatePSO(LPCWSTR shaderFile, LPCWSTR vertEntry, LPCWSTR pixEntry);
int UploadDrawData(std::span<UploadData> uploadData, Draws &draws);

void UseCamera(Camera &cam);
void RecordDraws(int rootSigIndex, int psoIndex, int vbufferIndex, UINT startIndex, UINT vertexCount);


}
