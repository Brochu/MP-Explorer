#pragma once
#include <d3d12.h>

namespace CmdManager {

void Init(ID3D12Device* device);
void Teardown();

}
