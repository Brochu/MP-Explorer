#pragma once
#include <d3d12.h>
#include <d3dx12.h>

namespace CmdManager {

enum QueueType {
    Graphics,
    Async,
    Copy,

    COUNT
};

void InitCmdManager(ID3D12Device* pDevice);
void ClearCmdManager();

void ClearCmdQueue(QueueType type);

void CreateCmdList(QueueType type);
uint64_t ExecuteCmdList(QueueType type, ID3D12CommandList *list);

uint64_t IncrementFence(QueueType type);
bool IsFenceComplete(QueueType type, uint64_t value);
void WaitForFence(QueueType type, uint64_t value);

}
