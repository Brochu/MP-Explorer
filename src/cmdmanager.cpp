#include "cmdmanager.h"
#include "cmdallocpool.h"

#include <mutex>

namespace CmdManager {
ID3D12Device *device;

struct CmdQueue {
    const D3D12_COMMAND_LIST_TYPE _type;
    ID3D12CommandQueue *_queue;

    CmdAllocPool _pool;
    std::mutex _fenceMutex;
    std::mutex _eventMutex;

    ID3D12Fence *_fence;
    uint64_t _nextValue;
    uint64_t _lastCompleteValue;
    HANDLE _fenceEvent;
};
CmdQueue GraphicsQueue { D3D12_COMMAND_LIST_TYPE_DIRECT };
CmdQueue ComputeQueue { D3D12_COMMAND_LIST_TYPE_COMPUTE };
CmdQueue CopyQueue { D3D12_COMMAND_LIST_TYPE_COPY };

void CreateQueue(CmdQueue &q, ID3D12Device *pdevice) {
    q._nextValue = (uint64_t)q._type << 56 | 1;
    q._lastCompleteValue = (uint64_t)q._type << 56;

    D3D12_COMMAND_QUEUE_DESC desc {};
    desc.Type = q._type;
    desc.NodeMask = 1;
    pdevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&q._queue));
    q._queue->SetName(L"CmdManager::CommandQueue");

    pdevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&q._fence));
    q._fence->SetName(L"CmdManager::Fence");
    q._fence->Signal((uint64_t)q._type << 56);

    q._fenceEvent = CreateEvent(nullptr, false, false, nullptr);

    CreateAllocPool(q._pool, q._type, pdevice);
}

void ClearQueue(CmdQueue &q) {
    if (q._queue == nullptr) {
        return;
    }

    ClearAllocPool(q._pool);
    CloseHandle(q._fenceEvent);

    q._fence->Release();
    q._fence = nullptr;

    q._queue->Release();
    q._queue = nullptr;
}

// ---------------------------------------------------------
void CreateCmdManager(ID3D12Device *pdevice) {
    device = pdevice;

    CreateQueue(GraphicsQueue, pdevice);
    CreateQueue(ComputeQueue, pdevice);
    CreateQueue(CopyQueue, pdevice);
}

void ClearCmdManager() {
    ClearQueue(GraphicsQueue);
    ClearQueue(ComputeQueue);
    ClearQueue(CopyQueue);
}

}
