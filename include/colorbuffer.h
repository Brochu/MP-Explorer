#pragma once
#include "pixelbuffer.h"

namespace Graphics {

struct ColorBuffer {
    PixelBuffer pix;
};

ColorBuffer CreateColorBuffer();

}
