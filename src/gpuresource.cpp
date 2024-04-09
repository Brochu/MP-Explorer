#include "gpuresource.h"

namespace Graphics {

GpuResource CreateGpuResource() {
    return {
        D3D12_RESOURCE_STATE_COMMON,
        (D3D12_RESOURCE_STATES)-1,
        D3D12_GPU_VIRTUAL_ADDRESS_NULL
    };
}

GpuResource CreateGpuResource(ID3D12Resource *res, D3D12_RESOURCE_STATES state) {
    return {
        state,
        (D3D12_RESOURCE_STATES)-1,
        D3D12_GPU_VIRTUAL_ADDRESS_NULL,
        res
    };
}

void DestroyGpuResource(GpuResource res) {
    res.pResource.Reset();
    res.gpuVAddr = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
    ++res.versionID;
}

ID3D12Resource *GpuResource::operator->() {
    return pResource.Get();
}

const ID3D12Resource *GpuResource::operator->() const {
    return pResource.Get();
}

}
