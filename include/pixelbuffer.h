#pragma once
#include "gpuresource.h"
#include <dxgi.h>

namespace Graphics {

DXGI_FORMAT GetBaseFormat(DXGI_FORMAT format);
DXGI_FORMAT GetUAVFormat(DXGI_FORMAT format);
DXGI_FORMAT GetDSVFormat(DXGI_FORMAT defaultFormat);
DXGI_FORMAT GetDepthFormat(DXGI_FORMAT defaultFormat);
DXGI_FORMAT GetStencilFormat(DXGI_FORMAT defaultFormat);
size_t BytesPerPixel(DXGI_FORMAT Format);

struct PixelBuffer {
    GpuResource res;

    uint32_t width;
    uint32_t height;
    uint32_t arraySize;
    DXGI_FORMAT format;
};

PixelBuffer CreatePixelBuffer();
D3D12_RESOURCE_DESC DescribeTex2D(PixelBuffer &buf, uint32_t width, uint32_t height,
                                  uint32_t arraySize, uint32_t numMips,
                                  DXGI_FORMAT format, UINT flags);
void AssocWithResource(PixelBuffer &buf, ID3D12Resource *res, D3D12_RESOURCE_STATES state);
void CreateTexResource(PixelBuffer &buf, ID3D12Device *device,
                       const D3D12_RESOURCE_DESC &desc, D3D12_CLEAR_VALUE clear);

}
