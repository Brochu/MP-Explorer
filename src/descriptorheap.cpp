#include "descriptorheap.h"
#include "graphics.h"
#include "utility.h"

#include <mutex>
#include <vector>

namespace Graphics {
using namespace Microsoft::WRL;

static const uint32_t s_numDescriptorPerHeap = 256;
static std::mutex s_allocMutex;
static std::vector<ComPtr<ID3D12DescriptorHeap>> s_descriptorHeapPool;

static ID3D12DescriptorHeap *RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) {
    std::lock_guard<std::mutex> lock(s_allocMutex);
    D3D12_DESCRIPTOR_HEAP_DESC desc { };
    desc.Type = type;
    desc.NumDescriptors = s_numDescriptorPerHeap;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 1;

    ComPtr<ID3D12DescriptorHeap> pHeap;
    ThrowIfFailed(g_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap)));
    s_descriptorHeapPool.emplace_back(pHeap);
    return pHeap.Get();
};

DescriptorAllocator MakeDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type) {
    //TODO: Impl
    return { type };
}

D3D12_CPU_DESCRIPTOR_HANDLE Allocate(DescriptorAllocator &descAllocator) {
    //TODO: Impl
    return (D3D12_CPU_DESCRIPTOR_HANDLE)0;
}
void DestroyAll(DescriptorAllocator &descAllocator) {
    //TODO: Impl
}

//--------------------
}
