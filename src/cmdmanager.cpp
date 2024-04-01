#include "cmdmanager.h"
#include "graphics.h"

#include <mutex>

namespace CmdManager {
using namespace Graphics;
using namespace Microsoft::WRL;

struct CmdQueue {
    ComPtr<ID3D12CommandQueue> queue;

    std::mutex fenceMutex;
    std::mutex eventMutex;
    ComPtr<ID3D12Fence> fence;

    uint64_t nextFenceValue;
    uint64_t lastCompleteFenceValue;
    HANDLE fenceEvent;
    //TODO: Each queue should have cmd allocator pool
};
CmdQueue queues[3];

void Init(ID3D12Device *device) {
    //TODO: Add asserts / ThrowIfFailed
    // Make graphhics common header with utilities
    CmdQueue &q = queues[QueueType::Graphics];
    q.nextFenceValue = 1;
    q.lastCompleteFenceValue = 0;

    D3D12_COMMAND_QUEUE_DESC desc { };
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 1;
    HRESULT hr = g_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&q.queue));

    hr = g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&q.fence));
    q.fence->Signal(0);

    q.fenceEvent = CreateEvent(nullptr, false, false, nullptr);
}

void Teardown() {
    for (int i = 0; i < QueueType::COUNT; i++) {
        Teardown((QueueType)i);
    }
}

void Teardown(QueueType type) {
    CmdQueue &q = queues[type];
    if (q.queue.Get() == nullptr) {
        return;
    }

    CloseHandle(q.fenceEvent);
    q.fence->Release();
    q.fence = nullptr;

    q.queue->Release();
    q.queue = nullptr;
}

uint64_t ExecuteCmdList(QueueType type, ID3D12CommandList *list) {
    CmdQueue &q = queues[type];
    std::lock_guard<std::mutex> LockGuard(q.fenceMutex);

    ((ID3D12GraphicsCommandList*)list)->Close();
    q.queue->ExecuteCommandLists(1, &list);
    q.queue->Signal(q.fence.Get(), q.nextFenceValue);

    return q.nextFenceValue++;
}

}
