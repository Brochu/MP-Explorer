#include "colorbuffer.h"

namespace Graphics {

ColorBuffer CreateColorBuffer() {
    ColorBuffer cbuf;
    cbuf.pix = CreatePixelBuffer();

    return cbuf;
}

}
