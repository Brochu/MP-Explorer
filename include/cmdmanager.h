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

void Init(ID3D12Device* device);
void Teardown();

void Teardown(QueueType type);

uint64_t ExecuteCmdList(ID3D12CommandList *list);

//TODO: Add functions to handle sync logic per queue

}
