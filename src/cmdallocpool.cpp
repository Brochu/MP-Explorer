#include "cmdallocpool.h"

namespace Graphics {

void InitPool(ID3D12Device *device, D3D12_COMMAND_LIST_TYPE type, CmdAllocPool &pool) {
    pool.type = type;
    for (int i = 0; i < MAX_ALLOCS; i++) {
        pool.readyList.push(i);
        device->CreateCommandAllocator(type, IID_PPV_ARGS(&pool.allocs[i]));
    }
}

ID3D12CommandAllocator *CmdAllocPool::RequestAlloc() {
    return nullptr;
}

void CmdAllocPool::DiscardAlloc() {
}

void CmdAllocPool::Clear() {
    for (int i = 0; i < MAX_ALLOCS; i++) {
        if (allocs[i] == nullptr) {
            continue;
        }

        allocs[i]->Release();
    }
}

}
