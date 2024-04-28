#pragma once
#include "descriptorheap.h"
#include <d3dx12.h>

namespace Graphics {

void Init(HWND hwnd, int width, int height);
void Teardown();

extern ID3D12Device *g_device;

extern DescriptorAllocator g_descriptorAllocator[];
inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count = 1) {
    return AllocDescriptors(g_descriptorAllocator[type], count);
}

}
