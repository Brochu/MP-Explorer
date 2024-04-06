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

bool QueueReady(CmdQueue &q) {
    return q.queue != nullptr;
}

uint64_t IncrementFence(CmdQueue &q) {
    std::lock_guard<std::mutex> lockGuard(q.fenceMutex);
    q.queue->Signal(q.fence, q.nextValue);
    return q.nextValue++;
}

bool IsFenceComplete(CmdQueue &q, uint64_t fenceValue) {
    if (fenceValue > q.lastCompleteValue) {
        q.lastCompleteValue = max(q.lastCompleteValue, q.fence->GetCompletedValue());
    }

    return fenceValue <= q.lastCompleteValue;
}

void StallForFence(CmdQueue &q, uint64_t fenceValue) {
    CmdQueue &producer = GetQueue((D3D12_COMMAND_LIST_TYPE)(fenceValue >> 56));
    q.queue->Wait(producer.fence, fenceValue);
}

void StallForProducer(CmdQueue &q, CmdQueue& producer) { 
    q.queue->Wait(producer.fence, producer.nextValue - 1);
}

void WaitForFence(CmdQueue &q, uint64_t fenceValue) {
    if (IsFenceComplete(q, fenceValue)) {
        return;
    }

    {
        std::lock_guard<std::mutex> lockGuard(q.eventMutex);

        q.fence->SetEventOnCompletion(fenceValue, q.fenceEvent);
        WaitForSingleObject(q.fenceEvent, INFINITE);
        q.lastCompleteValue = fenceValue;
    }
}

uint64_t ExecuteCommandList(CmdQueue &q, ID3D12CommandList *list) {
    std::lock_guard<std::mutex> lockGuard(q.fenceMutex);
    ((ID3D12GraphicsCommandList*)list)->Close();

    q.queue->ExecuteCommandLists(1, &list);
    q.queue->Signal(q.fence, q.nextValue);

    return q.nextValue++;
}
ID3D12CommandAllocator *RequestAllcoator(CmdQueue &q) {
    return nullptr;
}
void DiscardAllocator(CmdQueue &q, uint64_t value, ID3D12CommandAllocator *alloc) {
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

CmdQueue &GetGraphicsQueue() { return GraphicsQueue; }
CmdQueue &GetComputeQueue() { return ComputeQueue; }
CmdQueue &GetCopyQueue() { return CopyQueue; }

CmdQueue &GetQueue(D3D12_COMMAND_LIST_TYPE type) {
    if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE) {
        return ComputeQueue;
    }
    else if (type == D3D12_COMMAND_LIST_TYPE_COPY) {
        return CopyQueue;
    }
    else {
        return GraphicsQueue;
    }
}

ID3D12CommandQueue *GetCommandQueue() {
    return GraphicsQueue.queue;
}

void CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandList **list, ID3D12CommandAllocator **alloc) {
    //TODO: Create allocator + command list from that allocator
}

bool IsFenceComplete(uint64_t value) {
    return IsFenceComplete(GetQueue(D3D12_COMMAND_LIST_TYPE(value >> 56)), value);
}

void WaitForFence(uint64_t value) {
    CmdQueue &producer = GetQueue((D3D12_COMMAND_LIST_TYPE)(value >> 56));
    WaitForFence(producer, value);
}

void IdleGPU() {
    WaitForIdle(GraphicsQueue);
    WaitForIdle(ComputeQueue);
    WaitForIdle(CopyQueue);
}

}
