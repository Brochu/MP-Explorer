#include "cmdmanager.h"
#include "renderer.h"

#include <d3dx12.h>
#include <mutex>

namespace CmdManager {
using namespace Renderer;
using namespace Microsoft::WRL;

struct CmdQueue {
    ComPtr<ID3D12CommandQueue> queue;

    std::mutex fenceMutex;
    std::mutex eventMutex;
    ComPtr<ID3D12Fence> fence;

    uint64_t nextFenceValue;
    uint64_t lastCompleteFenceValue;
    HANDLE fenceEvent;
};
CmdQueue queues[3];

enum QueueType {
    Graphics,
    Async,
    Copy,

    COUNT
};

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
}

}
