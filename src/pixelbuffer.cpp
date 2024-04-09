#include "pixelbuffer.h"

namespace Graphics {

PixelBuffer CreatePixelBuffer() {
    PixelBuffer pbuf;
    pbuf.res = CreateGpuResource();

    pbuf.width = 0;
    pbuf.height = 0;
    pbuf.arraySize = 0;
    pbuf.format = DXGI_FORMAT_UNKNOWN;
    return pbuf;
}

}
