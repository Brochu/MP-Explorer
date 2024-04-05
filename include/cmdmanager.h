#pragma once
#include <d3d12.h>
#include <d3dx12.h>

namespace CmdManager {

void CreateCmdManager(ID3D12Device* pdevice);
void ClearCmdManager();

}
