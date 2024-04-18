#include "colorbuffer.h"

namespace Graphics {

ColorBuffer CreateColorBuffer(Color clear) {
    ColorBuffer cbuf;
    cbuf.pix = CreatePixelBuffer();
    cbuf.clearColor = clear;
    cbuf.numMipMaps = 0;
    cbuf.fragCount = 1;
    cbuf.sampleCount = 1;

    cbuf.srvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    cbuf.rtvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    for (size_t i = 0; i < _countof(cbuf.uavHandle); i++) {
        cbuf.uavHandle[i].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    return cbuf;
}

}
