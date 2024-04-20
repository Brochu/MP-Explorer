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
};

//--------------------
struct DescriptorHeap {
};

}
