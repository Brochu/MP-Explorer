#pragma once
#include <d3d12.h>

namespace CmdManager {

enum QueueType {
    Graphics,
    Async,
    Copy,

    COUNT
};

void Init(ID3D12Device* device);
void Teardown();

//TODO: Add functions to handle sync logic per queue

}
