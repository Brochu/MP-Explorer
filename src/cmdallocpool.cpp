#include "cmdallocpool.h"

namespace CmdManager {

void CreateAllocPool(CmdAllocPool &pool, D3D12_COMMAND_LIST_TYPE type, ID3D12Device *pDevice) {
    pool.type = type;
    pool.device = pDevice;
}
void ClearAllocPool(CmdAllocPool &pool) {
    for (size_t i = 0; i < pool.allocPool.size(); i++) {
        pool.allocPool[i]->Release();
    }

    pool.allocPool.clear();
}

ID3D12CommandAllocator *RequestAllocator(CmdAllocPool &pool, uint64_t completedValue) {
    std::lock_guard<std::mutex> lockGuard(pool.allocMutex);
    ID3D12CommandAllocator *alloc = nullptr;

    if (!pool.readyList.empty()) {
        std::pair<uint64_t, ID3D12CommandAllocator*> allocPair = pool.readyList.front();

        if (allocPair.first <= completedValue) {
            alloc = allocPair.second;
            alloc->Reset();
            pool.readyList.pop();
        }
    }

    if (alloc == nullptr) {
        pool.device->CreateCommandAllocator(pool.type, IID_PPV_ARGS(&alloc));
        pool.allocPool.push_back(alloc);
    }

    return alloc;
}
void DiscardAllocator(CmdAllocPool &pool, uint64_t value, ID3D12CommandAllocator *alloc) {
    std::lock_guard<std::mutex> lockGuard(pool.allocMutex);

    pool.readyList.push(std::make_pair(value, alloc));
}

}
