#include "descriptorheap.h"
#include "gpuresource.h"
#include "graphics.h"
#include "utility.h"

#include <assert.h>
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
    DescriptorAllocator alloc { type };
    alloc.heap = nullptr;
    alloc.descriptorSize = 0;
    alloc.numFreeHandles = 0;
    alloc.nextHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

    return alloc;
}

D3D12_CPU_DESCRIPTOR_HANDLE AllocDescriptors(DescriptorAllocator &allocator, uint32_t count) {
    if (allocator.heap == nullptr || allocator.numFreeHandles < count) {
        allocator.heap = RequestNewHeap(allocator.type);
        allocator.nextHandle = allocator.heap->GetCPUDescriptorHandleForHeapStart();
        allocator.numFreeHandles = s_numDescriptorPerHeap;

        if (allocator.descriptorSize == 0) {
            allocator.descriptorSize = g_device->GetDescriptorHandleIncrementSize(allocator.type);
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE res = allocator.nextHandle;
    allocator.nextHandle.ptr += count * allocator.descriptorSize;
    allocator.numFreeHandles -= count;
    return res;
}
void ClearDescriptorHeaps() {
    s_descriptorHeapPool.clear();
}

//--------------------
DescriptorHandle MakeDescriptorHandle() {
    DescriptorHandle handle;
    handle.cpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    handle.gpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    return handle;
}

DescriptorHandle MakeDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE pCpu, D3D12_GPU_DESCRIPTOR_HANDLE pGpu) {
    DescriptorHandle handle;
    handle.cpuHandle = pCpu;
    handle.gpuHandle = pGpu;
    return handle;
}

void OffsetHandleBy(DescriptorHandle &handle, INT offsetScaledByDescSize) {
    if (handle.cpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
        handle.cpuHandle.ptr += offsetScaledByDescSize;
    }

    if (handle.gpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
        handle.gpuHandle.ptr += offsetScaledByDescSize;
    }
}

size_t GetCpuPtr(DescriptorHandle &handle) {
    return handle.cpuHandle.ptr;
}
uint64_t GetGpuPtr(DescriptorHandle &handle) {
    return handle.gpuHandle.ptr;
}

bool IsHandleNull(DescriptorHandle &handle) {
    return handle.cpuHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}
bool IsHandleShaderVisible(DescriptorHandle &handle) {
    return handle.gpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}

//--------------------
DescriptorHeap MakeDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxCount) {
    DescriptorHeap res;
    res.heapDesc.Type = type;
    res.heapDesc.NumDescriptors = maxCount;
    res.heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // Do we need non-shader visible heaps?
    res.heapDesc.NodeMask = 1;

    ThrowIfFailed(g_device->CreateDescriptorHeap(&res.heapDesc, IID_PPV_ARGS(&res.ptr)));
    res.descriptorSize = g_device->GetDescriptorHandleIncrementSize(type);
    res.numFreeDescriptors = maxCount;
    res.firstHandle = {
        res.ptr->GetCPUDescriptorHandleForHeapStart(),
        res.ptr->GetGPUDescriptorHandleForHeapStart()
    };
    res.nextHandle = res.firstHandle;

    return res;
}
void DestroyDescriptorHeap(DescriptorHeap &heap) {
    heap.ptr->Release();
    heap.ptr = nullptr;
}

bool HasSpaceAvailable(DescriptorHeap &heap, uint32_t count) {
    return heap.numFreeDescriptors >= count;
}
DescriptorHandle AllocFromHeap(DescriptorHeap &heap, uint32_t count) {
    assert(HasSpaceAvailable(heap, count));

    DescriptorHandle res = heap.nextHandle;
    OffsetHandleBy(heap.nextHandle, count * heap.descriptorSize);
    heap.numFreeDescriptors -= count;
    return res;
}

DescriptorHandle GetFromHeap(DescriptorHeap &heap, uint32_t index) {
    DescriptorHandle handle = heap.firstHandle;
    OffsetHandleBy(handle, index * heap.descriptorSize);
    return handle;
}

uint32_t GetOffsetOfHandle(DescriptorHeap &heap, DescriptorHandle &handle) {
    return (uint32_t)(GetCpuPtr(handle) - GetCpuPtr(heap.firstHandle)) / heap.descriptorSize;
}

bool ValidateHandle(DescriptorHeap &heap, DescriptorHandle &handle) {
    if (GetCpuPtr(handle) < GetCpuPtr(heap.firstHandle) || 
        GetCpuPtr(handle) >= GetCpuPtr(heap.firstHandle) + heap.heapDesc.NumDescriptors * heap.descriptorSize)
    {
        return false;
    }

    if (GetGpuPtr(handle) - GetGpuPtr(heap.firstHandle) != GetCpuPtr(handle) - GetCpuPtr(heap.firstHandle)) {
        return false;
    }

    return true;
}

ID3D12DescriptorHeap *GetHeapPointer(DescriptorHeap &heap) {
    return heap.ptr.Get();
}

}
