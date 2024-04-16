#include "colorbuffer.h"

namespace Graphics {

ColorBuffer CreateColorBuffer(Color clear) {
    ColorBuffer cbuf;
    cbuf.pix = CreatePixelBuffer();
    cbuf.clearColor = clear;

    return cbuf;
}

}
