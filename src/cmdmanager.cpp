#include "cmdmanager.h"
#include "cmdallocpool.h"
#include "graphics.h"

#include <mutex>

namespace CmdManager {
using namespace Graphics;
using namespace Microsoft::WRL;

ID3D12Device *device;

struct CmdQueue {
    ComPtr<ID3D12CommandQueue> queue;

    std::mutex fenceMutex;
    std::mutex eventMutex;
    ComPtr<ID3D12Fence> fence;

    uint64_t nextFenceValue;
    uint64_t lastCompleteFenceValue;
    HANDLE fenceEvent;

    CmdAllocPool pool;
};
CmdQueue queues[QueueType::COUNT];

void InitCmdManager(ID3D12Device *pDevice) {
    device = pDevice;
    //TODO: Add asserts / ThrowIfFailed
    // Make graphhics common header with utilities
    CmdQueue &q = queues[QueueType::Graphics];
    q.nextFenceValue = (uint64_t)QueueType::Graphics << 56 | 1;
    q.lastCompleteFenceValue = (uint64_t)QueueType::Graphics << 56;

    D3D12_COMMAND_QUEUE_DESC desc { };
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 1;
    HRESULT hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&q.queue));

    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&q.fence));
    q.fence->Signal((uint64_t)QueueType::Graphics << 56);

    q.fenceEvent = CreateEvent(nullptr, false, false, nullptr);

    CreateAllocPool(q.pool, desc.Type, device);
}

void ClearCmdManager() {
    for (int i = 0; i < QueueType::COUNT; i++) {
        ClearCmdQueue((QueueType)i);
    }
}

void ClearCmdQueue(QueueType type) {
    CmdQueue &q = queues[type];
    if (q.queue.Get() == nullptr) {
        return;
    }

    ClearAllocPool(q.pool);

    CloseHandle(q.fenceEvent);
    q.fence->Release();
    q.fence = nullptr;

    q.queue->Release();
    q.queue = nullptr;
}

void CreateCmdList(QueueType type) {
    //IMPL
    // Depends on implementing command allocators per queue
}

uint64_t ExecuteCmdList(QueueType type, ID3D12CommandList *list) {
    CmdQueue &q = queues[type];
    std::lock_guard<std::mutex> LockGuard(q.fenceMutex);

    ((ID3D12GraphicsCommandList*)list)->Close();
    q.queue->ExecuteCommandLists(1, &list);
    q.queue->Signal(q.fence.Get(), q.nextFenceValue);

    return q.nextFenceValue++;
}

uint64_t IncrementFence(QueueType type) {
    CmdQueue &q = queues[type];
    std::lock_guard<std::mutex> LockGuard(q.fenceMutex);

    q.queue->Signal(q.fence.Get(), q.nextFenceValue);
    return q.nextFenceValue++;
}

bool IsFenceComplete(QueueType type, uint64_t value) {
    CmdQueue &q = queues[type];

    if (value > q.lastCompleteFenceValue) {
        q.lastCompleteFenceValue = max(q.lastCompleteFenceValue, q.fence->GetCompletedValue());
    }
    return value <= q.lastCompleteFenceValue;
}

void WaitForFence(QueueType type, uint64_t value) {
    if (IsFenceComplete(type, value)) {
        return;
    }

    CmdQueue &q = queues[type];
    {
        std::lock_guard<std::mutex> lockGuard(q.eventMutex);

        q.fence->SetEventOnCompletion(value, q.fenceEvent);
        WaitForSingleObject(q.fenceEvent, INFINITE);
        q.lastCompleteFenceValue = value;
    }
}

}
