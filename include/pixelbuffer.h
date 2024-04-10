#pragma once
#include "gpuresource.h"
#include <dxgi.h>

namespace Graphics {

DXGI_FORMAT GetBaseFormat(DXGI_FORMAT format);
DXGI_FORMAT GetUAVFormat(DXGI_FORMAT format);
DXGI_FORMAT GetDSVFormat(DXGI_FORMAT defaultFormat);
DXGI_FORMAT GetDepthFormat(DXGI_FORMAT defaultFormat);
DXGI_FORMAT GetStencilFormat(DXGI_FORMAT defaultFormat);

struct PixelBuffer {
    GpuResource res;

    uint32_t width;
    uint32_t height;
    uint32_t arraySize;
    DXGI_FORMAT format;
};

PixelBuffer CreatePixelBuffer();

}
