#include "cmdmanager.h"
#include <d3dx12.h>

namespace CmdManager {
using namespace Microsoft::WRL;

ComPtr<ID3D12CommandQueue> *queue;

void Init(ID3D12Device *device) {
    //TODO: Create main command queue
}

void Teardown() {
}

}
