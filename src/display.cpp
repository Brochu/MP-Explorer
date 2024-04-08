#include "display.h"
#include "cmdmanager.h"
#include "utility.h"

#include <dxgi1_6.h>

#define SWAPCHAIN_BUFFER_COUNT 3

DXGI_FORMAT swapchainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
IDXGISwapChain1 *swapchain;

namespace Display {
using namespace Microsoft::WRL;
using namespace Graphics;

void Init(HWND hwnd) {
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));

    DXGI_SWAP_CHAIN_DESC1 swapDesc {};
    swapDesc.Width = g_displayWidth;
    swapDesc.Height = g_displayHeight;
    swapDesc.Format = swapchainFormat;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.BufferCount = SWAPCHAIN_BUFFER_COUNT;
    swapDesc.SampleDesc = { 1, 0 }; // Count, Quality
    swapDesc.Scaling = DXGI_SCALING_NONE;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc {};
    fullscreenDesc.Windowed = TRUE;

    ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
        CmdManager::GetCommandQueue(),
        hwnd,
        &swapDesc, &fullscreenDesc,
        nullptr, &swapchain
    ));

    for (int i = 0; i < SWAPCHAIN_BUFFER_COUNT; i++) {
        //TODO: Get swapchain RT resources
        // Create RTVs for the different backbuffers in the swapchain
    }
}

void Teardown() {
}

void Resize(uint32_t width, uint32_t height) {
}

void Present() {
}

}

namespace Graphics {

uint32_t g_displayWidth = 800;
uint32_t g_displayHeight = 600;

uint64_t GetFrameCount() {
    return 0;
}

float GetFrameTime() {
    return 0.f;
}

float GetFrameRate() {
    return 0.f;
}

}
