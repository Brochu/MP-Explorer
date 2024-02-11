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

struct D3D12_ROOT_PARAMETER;
struct D3D12_STATIC_SAMPLER_DESC;

namespace Render {

void Setup(HWND hwnd, int width, int height);
void StartFrame(std::span<D3D12_VIEWPORT> viewports, std::span<D3D12_RECT> scissors, int rootSigIndex, int psoIndex);
void EndFrame();
void Teardown();

int CreateRootSignature(std::span<D3D12_ROOT_PARAMETER> params, std::span<D3D12_STATIC_SAMPLER_DESC> samplers);
int CreatePSO(LPCWSTR shaderFile, LPCWSTR vertEntry, LPCWSTR pixEntry);
int CreateBufferedCB(size_t bufferSize);
Draws UploadDrawData(std::span<UploadData> uploadData);

void BindBufferedCB(int CBIndex, void *data, size_t length);
void RecordDraws(UINT idxCount, UINT idxStart, INT vertOffset);

}
