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
//TODO: Maybe we don't need this anymore
void StartFrame(UINT originX, UINT originY, UINT width, UINT height, int rootSigIndex, int psoIndex);
//TODO Still needed to handle UI and draw + move to next frame
void EndFrame();
void Teardown();

//TODO: Should only have one root sig: bindless model
int CreateRootSignature(std::span<RootSigParam> params, std::span<RootSigSample> samplers);
int CreatePSO(LPCWSTR shaderFile, LPCWSTR vertEntry, LPCWSTR pixEntry);
//TODO: Might be able to abstract CB, be able to create SRVs and UAVs
int CreateBufferedCB(size_t bufferSize);
Draws UploadDrawData(std::span<UploadData> uploadData);

//TODO: Move functions to impact the current command list with commands
void BindBufferedCB(int CBIndex, void *data, size_t length);
void RecordDraws(UINT idxCount, UINT idxStart, INT vertOffset);

}
