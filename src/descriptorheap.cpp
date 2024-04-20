#include "descriptorheap.h"

#include <mutex>
#include <vector>

namespace Graphics {
using namespace Microsoft::WRL;

static const uint32_t s_numDescriptorPerHeap = 256;
static std::mutex s_allocMutex;
static std::vector<ComPtr<ID3D12DescriptorHeap>> s_descriptorHeapPool;

static ID3D12DescriptorHeap *RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) {
    //TODO: Impl
    return nullptr;
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
