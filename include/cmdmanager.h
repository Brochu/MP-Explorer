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

CmdQueue &GetGraphicsQueue();
CmdQueue &GetComputeQueue();
CmdQueue &GetCopyQueue();

CmdQueue &GetQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
ID3D12CommandQueue *GetCommandQueue();

void CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandList **list, ID3D12CommandAllocator **alloc);

bool IsFenceComplete(uint64_t value);
void WaitForFence(uint64_t value);
void IdleGPU();

//-------------------------------------------------------------
bool QueueReady(CmdQueue &q);

uint64_t IncrementFence(CmdQueue &q);
bool IsFenceComplete(CmdQueue &q, uint64_t fenceValue);
void StallForFence(CmdQueue &q, uint64_t fenceValue);
void StallForProducer(CmdQueue &q, CmdQueue& producer);
void WaitForFence(CmdQueue &q, uint64_t fenceValue);
inline void WaitForIdle(CmdQueue &q) { WaitForFence(q, IncrementFence(q)); }

inline ID3D12CommandQueue* GetCommandQueue(CmdQueue &q) { return q.queue; }

inline uint64_t GetNextFenceValue(CmdQueue &q) { return q.nextValue; }

}
