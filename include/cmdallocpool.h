#pragma once
#include <d3d12.h>

namespace Graphics {

struct CmdAllocPool {
    ID3D12CommandAllocator *RequestAlloc();
    void DiscardAlloc();

    void Clear();
};

CmdAllocPool CreateAllocatorPool(D3D12_COMMAND_LIST_TYPE type);

}
