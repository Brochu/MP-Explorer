#pragma once
#include <span>

#include <d3d12.h>
#include <DirectXMath.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
void Teardown();

void CreateRootSignature(std::span<RootSigParam> params, std::span<RootSigSample> samplers);
size_t CreatePSO(LPCWSTR path);
size_t UploadVertBuffer(void *data, size_t size, size_t stride);

}
