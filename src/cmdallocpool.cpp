#include "cmdallocpool.h"

namespace Graphics {

CmdAllocPool CreateAllocatorPool(D3D12_COMMAND_LIST_TYPE type) {
    return {};
}

ID3D12CommandAllocator *CmdAllocPool::RequestAlloc() {
    return nullptr;
}

void CmdAllocPool::DiscardAlloc() {
}

void CmdAllocPool::Clear() {
}

}
