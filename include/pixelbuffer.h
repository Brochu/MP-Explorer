#pragma once
#include "gpuresource.h"
#include <dxgi.h>

namespace Graphics {

struct PixelBuffer {
    GpuResource res;

    uint32_t width;
    uint32_t height;
    uint32_t arraySize;
    DXGI_FORMAT format;
};

PixelBuffer CreatePixelBuffer();

}
