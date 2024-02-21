#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d12.h>
#include <DirectXMath.h>

#include <span>
#include <vector>

//TODO: Need to look into moving this away from the renderer
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

struct RootSigParam {
    enum Type { None, SRVDescriptor, CBVDescriptor, UAVDescriptor };
    Type descriptorType;
    UINT descriptorIndex;

    //TODO: Add parameters for descriptor tables
    //TODO: Add parameters for constants
};

struct RootSigSample {
    //TODO: To fill in
};

namespace Render {

void Setup(HWND hwnd, int width, int height);
void StartFrame(UINT originX, UINT originY, UINT width, UINT height, int rootSigIndex, int psoIndex);
void EndFrame();
void Teardown();

int CreateRootSignature(std::span<RootSigParam> params, std::span<RootSigSample> samplers);
int CreatePSO(LPCWSTR shaderFile, LPCWSTR vertEntry, LPCWSTR pixEntry);
int CreateBufferedCB(size_t bufferSize);
Draws UploadDrawData(std::span<UploadData> uploadData);

void BindBufferedCB(int CBIndex, void *data, size_t length);
void RecordDraws(UINT idxCount, UINT idxStart, INT vertOffset);

}
