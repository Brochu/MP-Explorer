#pragma once
#include "descriptorheap.h"
#include <d3d12.h>

namespace Graphics {
extern ID3D12Device *g_device;
extern DescriptorAllocator g_descriptorAllocator[];

void Init(HWND hwnd, int width, int height);
void Teardown();

}
