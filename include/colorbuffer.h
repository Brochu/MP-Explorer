#pragma once
#include "pixelbuffer.h"
#include "color.h"

namespace Graphics {

struct ColorBuffer {
    PixelBuffer pix;

    Color clearColor;
};

ColorBuffer CreateColorBuffer(Color clear = MakeColor(0.f, 0.f, 0.f, 0.f));

}
