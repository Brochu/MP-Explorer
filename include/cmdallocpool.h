#pragma once
#include <d3d12.h>
#include <mutex>
#include <queue>

namespace Graphics {
#define MAX_ALLOCS 10

struct CmdAllocPool {
    ID3D12CommandAllocator *RequestAlloc();
    void DiscardAlloc();

    void Clear();

    D3D12_COMMAND_LIST_TYPE type;
    ID3D12CommandAllocator *allocs[MAX_ALLOCS];

    std::queue<size_t> readyList;
    std::mutex allocMutex;
};

void InitPool(ID3D12Device *device, D3D12_COMMAND_LIST_TYPE type, CmdAllocPool &pool);

}
