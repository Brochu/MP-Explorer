#pragma once
#include "pixelbuffer.h"
#include "color.h"

namespace Graphics {

struct ColorBuffer {
    PixelBuffer pix;

    Color clearColor;
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE uavHandle[12];
    uint32_t numMipMaps;
    uint32_t fragCount;
    uint32_t sampleCount;
};

ColorBuffer CreateColorBuffer(Color clear = MakeColor(0.f, 0.f, 0.f, 0.f));

}
