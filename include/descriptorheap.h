#pragma once
#include <d3dx12.h>

namespace Graphics {

struct DescriptorAllocator {
    D3D12_DESCRIPTOR_HEAP_TYPE type;
    ID3D12DescriptorHeap *heap;
    D3D12_CPU_DESCRIPTOR_HANDLE nextHandle;
    uint32_t descriptorSize;
    uint32_t numFreeHandles;
};

DescriptorAllocator MakeDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type);
D3D12_CPU_DESCRIPTOR_HANDLE AllocDescriptors(DescriptorAllocator &descAllocator, uint32_t count);
void ClearDescriptorHeaps();

//--------------------
struct DescriptorHandle {
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
};

DescriptorHandle MakeDescriptorHandle();
DescriptorHandle MakeDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE pCpu, D3D12_GPU_DESCRIPTOR_HANDLE pGpu);
void OffsetHandleBy(DescriptorHandle &handle, INT offsetScaledByDescSize);
size_t GetCpuPtr(DescriptorHandle &handle);
uint64_t GetGpuPtr(DescriptorHandle &handle);
bool IsHandleNull(DescriptorHandle &handle);
bool IsHandleShaderVisible(DescriptorHandle &handle);

//--------------------
struct DescriptorHeap {
};

}
