#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi.h>
#include <dxgi1_4.h>

#include <cassert>
#include <stdio.h>
#include <wrl.h>

#define FRAME_COUNT 3

namespace Render {
using namespace Microsoft::WRL;

D3D12_VIEWPORT viewport;
D3D12_RECT scissor;
int32_t rtvDescSize;

ComPtr<ID3D12Device> device;
ComPtr<ID3D12CommandQueue> queue;
ComPtr<IDXGISwapChain> swapchain;
ComPtr<ID3D12CommandAllocator> cmdallocs[FRAME_COUNT];
ComPtr<ID3D12GraphicsCommandList> cmdlists[FRAME_COUNT];
ComPtr<ID3D12Resource> rts[FRAME_COUNT];
ComPtr<ID3D12DescriptorHeap> rtheap;

uint64_t currentfenceval;
uint64_t fencevals[FRAME_COUNT];
HANDLE fenceevts[FRAME_COUNT];
ComPtr<ID3D12Fence> framefence[FRAME_COUNT];

void setup(HWND hwnd, int width, int height) {
    ComPtr<ID3D12Debug> debugCtrl;
    D3D12GetDebugInterface(IID_PPV_ARGS(&debugCtrl));
    debugCtrl->EnableDebugLayer();

    D3D_FEATURE_LEVEL minlvl = D3D_FEATURE_LEVEL_12_0;
    HRESULT hr = D3D12CreateDevice(nullptr, minlvl, IID_PPV_ARGS(&device));
    if (FAILED(hr)) {
        printf("[RENDER] Dx12 device creation failed (%ld).\n", hr);
        assert(false);
    }

    D3D12_COMMAND_QUEUE_DESC qdesc {};
    qdesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    hr = device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&queue));
    if (FAILED(hr)) {
        printf("[RENDER] Dx12 command queue creation failed (%ld).\n", hr);
        assert(false);
    }

    ComPtr<IDXGIFactory4> factory;
    hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        printf("[RENDER] Dx12 dxgi factory creation failed (%ld).\n", hr);
        assert(false);
    }

    DXGI_SWAP_CHAIN_DESC swapdesc {};
    ZeroMemory(&swapdesc, sizeof(swapdesc));
    swapdesc.BufferCount = FRAME_COUNT;
    swapdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapdesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapdesc.BufferDesc.Width = width;
    swapdesc.BufferDesc.Height = height;
    swapdesc.SampleDesc.Count = 1;
    swapdesc.OutputWindow = hwnd;
    swapdesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapdesc.Windowed = true;
    hr = factory->CreateSwapChain(queue.Get(), &swapdesc, &swapchain);
    if (FAILED(hr)) {
        printf("[RENDER] Dx12 swapchain creation failed (%ld).\n", hr);
        assert(false);
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapdesc {};
    heapdesc.NumDescriptors = FRAME_COUNT;
    heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&rtheap));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvhandle { rtheap->GetCPUDescriptorHandleForHeapStart() };
    rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (int i = 0; i < FRAME_COUNT; i++) {
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdallocs[i]));
        device->CreateCommandList(0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmdallocs[i].Get(),
            nullptr,
            IID_PPV_ARGS(&cmdlists[i])
        );
        cmdlists[i]->Close();
        // ----------------------------------------
        fenceevts[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        fencevals[i] = 0;
        device->CreateFence(fencevals[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&framefence[i]));
        // ----------------------------------------

        swapchain->GetBuffer(i, IID_PPV_ARGS(&rts[i]));
        D3D12_RENDER_TARGET_VIEW_DESC rtvdesc {};
        rtvdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvdesc.Texture2D.MipSlice = 0;
        rtvdesc.Texture2D.PlaneSlice = 0;

        device->CreateRenderTargetView(rts[i].Get(), &rtvdesc, rtvhandle);
        rtvhandle.Offset(rtvDescSize);
    }

    currentfenceval = 1;
    scissor = { 0, 0, width, height };
    viewport = { 0.f, 0.f, (float)width, (float)height, 0.f, 1.f };
}

void frame() {
    //TODO: Up next, need to move the preparerender logic here
}

void teardown() {
    //TODO: Go through objects created in setup and free in reverse order
}

}
