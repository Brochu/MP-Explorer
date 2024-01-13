#include "renderer.h"

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h> //TODO: Remove once we get more recent DXC version
#include <dxcapi.h>

#include <cassert>
#include <stdio.h>
#include <wrl.h>

#define FRAME_COUNT 2

namespace Render {
using namespace Microsoft::WRL;

CD3DX12_VIEWPORT viewport;
CD3DX12_RECT scissor;
UINT rtvDescSize;

ComPtr<ID3D12Device> device;
ComPtr<ID3D12CommandQueue> queue;
ComPtr<ID3D12CommandAllocator> cmdAllocs[FRAME_COUNT];
ComPtr<ID3D12GraphicsCommandList> cmdLists[FRAME_COUNT];
ComPtr<IDXGISwapChain4> swapchain;
ComPtr<ID3D12DescriptorHeap> rtvheap;
ComPtr<ID3D12Resource> rtvs[FRAME_COUNT];

ComPtr<ID3D12RootSignature> root;
ComPtr<ID3D12PipelineState> pso;

UINT frameIndex;
HANDLE fenceEvent;
ComPtr<ID3D12Fence> fence;
UINT64 fenceValues[FRAME_COUNT];

inline void ThrowIfFailed(HRESULT hr);
void GetHardwareAdapter(IDXGIFactory1 *pfactory, IDXGIAdapter1 **ppAdapter, bool hpAdapter);
void WaitForGPU();
void MoveToNextFrame();
HRESULT CompileShader(LPCWSTR file, LPCSTR entry, LPCSTR target, ComPtr<ID3DBlob> &shader);

void setup(HWND hwnd, int width, int height) {
    printf("[R-START] Preparing renderer.\n");
    frameIndex = 0;
    viewport = CD3DX12_VIEWPORT(0.f, 0.f, (float)width, (float)height);
    scissor = CD3DX12_RECT(0, 0, width, height);
    rtvDescSize = 0;

    for (int i = 0; i < FRAME_COUNT; i++) {
        fenceValues[i] = 0;
    }
    //-------------------------
    UINT factoryflags = 0;
    ComPtr<ID3D12Debug> debugCtrl;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugCtrl)))) {
        debugCtrl->EnableDebugLayer();
        factoryflags |= DXGI_CREATE_FACTORY_DEBUG;
    }
    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(factoryflags, IID_PPV_ARGS(&factory)));

    ComPtr<IDXGIAdapter1> hardware;
    GetHardwareAdapter(factory.Get(), &hardware, true);
    ThrowIfFailed(D3D12CreateDevice(hardware.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));
    //-------------------------
    D3D12_COMMAND_QUEUE_DESC qdesc {};
    qdesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&queue)));
    //-------------------------
    DXGI_SWAP_CHAIN_DESC1 swapdesc {};
    swapdesc.BufferCount = FRAME_COUNT;
    swapdesc.Width = width;
    swapdesc.Height = height;
    swapdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapdesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapdesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swap;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        queue.Get(),
        hwnd,
        &swapdesc,
        nullptr,
        nullptr,
        &swap
    ));
    ThrowIfFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swap.As(&swapchain));
    frameIndex = swapchain->GetCurrentBackBufferIndex();
    //-------------------------
    { // Descriptor Heaps
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc {};
        rtvHeapDesc.NumDescriptors = FRAME_COUNT;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvheap)));
        rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }
    //-------------------------
    { // Frame Resources
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvhandle(rtvheap->GetCPUDescriptorHandleForHeapStart());

        for (UINT i = 0; i < FRAME_COUNT; i++) {
            ThrowIfFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&rtvs[i])));
            device->CreateRenderTargetView(rtvs[i].Get(), nullptr, rtvhandle);
            rtvhandle.Offset(1, rtvDescSize);

            ThrowIfFailed(device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&cmdAllocs[i])
            ));
        }
    }
    //-------------------------
    { // Sync Objects
        ThrowIfFailed(device->CreateFence(fenceValues[frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        fenceValues[frameIndex]++;

        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fenceEvent == nullptr) {
            assert(false);
        }
    }
    for (int i = 0; i < FRAME_COUNT; i++) {
        ThrowIfFailed(device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmdAllocs[i].Get(),
            nullptr,
            IID_PPV_ARGS(&cmdLists[i])
        ));
        ThrowIfFailed(cmdLists[i]->Close());
    }
    { // Empty Root Signature
        CD3DX12_ROOT_SIGNATURE_DESC rootdesc {};
        rootdesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> sign;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootdesc, D3D_ROOT_SIGNATURE_VERSION_1, &sign, &error));
        ThrowIfFailed(device->CreateRootSignature(
            0,
            sign->GetBufferPointer(),
            sign->GetBufferSize(),
            IID_PPV_ARGS(&root)
        ));
    }
    { // Creating PSO
        //TODO: Look into using more recent shader compiler
        LPCWSTR file = L"shaders\\shaders.hlsl";
        ComPtr<ID3DBlob> vs;
        ComPtr<ID3DBlob> ps;
        CompileShader(file, "VSMain", "vs_5_1", vs);
        CompileShader(file, "PSMain", "ps_5_1", ps);

        D3D12_INPUT_ELEMENT_DESC inputElem[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psodesc = { 0 };
        psodesc.InputLayout = { inputElem, _countof(inputElem) };
        psodesc.pRootSignature = root.Get();
        psodesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
        psodesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
        psodesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psodesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psodesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psodesc.DepthStencilState.DepthEnable = FALSE;
        psodesc.DepthStencilState.StencilEnable = FALSE;
        psodesc.SampleMask = UINT_MAX;
        psodesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psodesc.NumRenderTargets = 1;
        psodesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psodesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(&psodesc, IID_PPV_ARGS(&pso)));
    }
}

void frame() {
    ThrowIfFailed(cmdAllocs[frameIndex]->Reset());
    ThrowIfFailed(cmdLists[frameIndex]->Reset(cmdAllocs[frameIndex].Get(), pso.Get()));
    //TODO: Render logic here
    ThrowIfFailed(cmdLists[frameIndex]->Close());
}

void teardown() {
    printf("[R-STOP] Teardown renderer.\n");

    WaitForGPU();
    CloseHandle(fenceEvent);
}

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        assert(false);
    }
}

void GetHardwareAdapter(IDXGIFactory1 *pfactory, IDXGIAdapter1 **ppAdapter, bool hpAdapter) {
    *ppAdapter = nullptr;
    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<IDXGIFactory6> factory;

    if (SUCCEEDED(pfactory->QueryInterface(IID_PPV_ARGS(&factory)))) {
        for (UINT adapterIndex = 0;
             SUCCEEDED(factory->EnumAdapterByGpuPreference(
                 adapterIndex,
                 hpAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                 IID_PPV_ARGS(&adapter)));
             adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr))) {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr) {
        for (UINT adapterIndex = 0; SUCCEEDED(pfactory->EnumAdapters1(adapterIndex, &adapter)); adapterIndex++) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr))) {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}

void WaitForGPU() {
    ThrowIfFailed(queue->Signal(fence.Get(), fenceValues[frameIndex]));
    ThrowIfFailed(fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent));
    WaitForSingleObjectEx(fenceEvent, INFINITE, false);

    fenceValues[frameIndex]++;
}

void MoveToNextFrame() {
    const UINT64 currentFenceValue = fenceValues[frameIndex];
    ThrowIfFailed(queue->Signal(fence.Get(), currentFenceValue));

    frameIndex = swapchain->GetCurrentBackBufferIndex();

    if (fence->GetCompletedValue() < fenceValues[frameIndex]) {
        ThrowIfFailed(fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent));
        WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
    }

    fenceValues[frameIndex] = currentFenceValue + 1;
}

HRESULT CompileShader(LPCWSTR file, LPCSTR entry, LPCSTR target, ComPtr<ID3DBlob> &shader) {
    printf("[SHADER] file = '%ls'\n", (wchar_t*)file);
    printf("[SHADER] entry = '%s'\n", (char*)entry);
    printf("[SHADER] target = '%s'\n", (char*)target);

    UINT compileflags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3DCompileFromFile(file, nullptr, nullptr, entry, target, compileflags, 0, &shader, &error);
    if (FAILED(hr)) {
        //TODO: Show error in console
        ThrowIfFailed(hr);
    }
    else {
        const char *code = (const char*)shader->GetBufferPointer();
        printf("[SHADER] code = \n%s\n\n", code);
    }
    return hr;
}

}
