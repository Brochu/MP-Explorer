#pragma once
#include <d3dx12.h>

namespace Graphics {
#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

struct GpuResource {
    D3D12_RESOURCE_STATES usageState;
    D3D12_RESOURCE_STATES transitionState;
    D3D12_GPU_VIRTUAL_ADDRESS gpuVAddr;
    Microsoft::WRL::ComPtr<ID3D12Resource> pResource;

    uint32_t versionID = 0;

    ID3D12Resource *operator->();
    const ID3D12Resource *operator->() const;
};

GpuResource CreateGpuResource();
GpuResource CreateGpuResource(ID3D12Resource *res, D3D12_RESOURCE_STATES state);

void DestroyGpuResource(GpuResource &res);

inline ID3D12Resource *GetResource(GpuResource res) { return res.pResource.Get(); }
inline ID3D12Resource **GetAddressOf(GpuResource res) { return res.pResource.GetAddressOf(); }
inline D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddr(GpuResource res) { return res.gpuVAddr; }
inline uint32_t GetVersionID(GpuResource res) { return res.versionID; }

}
