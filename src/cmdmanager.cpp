#include "cmdmanager.h"

namespace CmdManager {
ID3D12Device *device;

CmdQueue GraphicsQueue { D3D12_COMMAND_LIST_TYPE_DIRECT };
CmdQueue ComputeQueue { D3D12_COMMAND_LIST_TYPE_COMPUTE };
CmdQueue CopyQueue { D3D12_COMMAND_LIST_TYPE_COPY };

void CreateQueue(CmdQueue &q, ID3D12Device *pdevice) {
    q.nextValue = (uint64_t)q.type << 56 | 1;
    q.lastCompleteValue = (uint64_t)q.type << 56;

    D3D12_COMMAND_QUEUE_DESC desc {};
    desc.Type = q.type;
    desc.NodeMask = 1;
    pdevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&q.queue));
    q.queue->SetName(L"CmdManager::CommandQueue");

    pdevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&q.fence));
    q.fence->SetName(L"CmdManager::Fence");
    q.fence->Signal((uint64_t)q.type << 56);

    q.fenceEvent = CreateEvent(nullptr, false, false, nullptr);

    CreateAllocPool(q.pool, q.type, pdevice);
}

void ClearQueue(CmdQueue &q) {
    if (q.queue == nullptr) {
        return;
    }

    ClearAllocPool(q.pool);
    CloseHandle(q.fenceEvent);

    q.fence->Release();
    q.fence = nullptr;

    q.queue->Release();
    q.queue = nullptr;
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
