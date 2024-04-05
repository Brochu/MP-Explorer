#pragma once

#include "cmdallocpool.h"
#include <mutex>

#include <d3d12.h>
#include <d3dx12.h>

namespace CmdManager {

struct CmdQueue {
    const D3D12_COMMAND_LIST_TYPE type;
    ID3D12CommandQueue *queue;

    CmdAllocPool pool;
    std::mutex fenceMutex;
    std::mutex eventMutex;

    ID3D12Fence *fence;
    uint64_t nextValue;
    uint64_t lastCompleteValue;
    HANDLE fenceEvent;
};

void CreateCmdManager(ID3D12Device* pdevice);
void ClearCmdManager();

}
