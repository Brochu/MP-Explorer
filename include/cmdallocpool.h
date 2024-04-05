#pragma once
#include <d3d12.h>
#include <mutex>
#include <queue>
#include <vector>

namespace CmdManager {
#define MAX_ALLOCS 10

struct CmdAllocPool {
    D3D12_COMMAND_LIST_TYPE type;
    ID3D12Device *device;

    std::vector<ID3D12CommandAllocator*> allocPool;
    std::queue<size_t> readyList;
    std::mutex allocMutex;
};

void CreateAllocPool(CmdAllocPool &pool, ID3D12Device *pDevice);
void ClearAllocPool(CmdAllocPool &pool);

ID3D12CommandAllocator *RequestAllocator(CmdAllocPool &pool, uint64_t completedValue);
void DiscardAllocator(CmdAllocPool &pool, uint64_t value, ID3D12CommandAllocator *alloc);

inline size_t AllocPoolSize(CmdAllocPool &pool) { return pool.allocPool.size(); }

}
