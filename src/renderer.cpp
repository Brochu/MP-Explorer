#include "renderer.h"
#include "debugui.h"
#include "TracyD3D12.hpp"

#include <d3dx12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <d3d12shader.h>

#include <cassert>
#include <wrl.h>

#define FRAME_COUNT 2

namespace Render {
using namespace Microsoft::WRL;
using namespace DirectX;

UINT rtvDescSize;
UINT dsvDescSize;
UINT resDescSize;

ComPtr<ID3D12Device> device;
ComPtr<ID3D12CommandQueue> queue;
ComPtr<ID3D12CommandAllocator> cmdAllocs[FRAME_COUNT];
ComPtr<ID3D12GraphicsCommandList> cmdLists[FRAME_COUNT];
ComPtr<IDXGISwapChain4> swapchain;

ComPtr<ID3D12DescriptorHeap> rtvheap;
ComPtr<ID3D12Resource> rtvs[FRAME_COUNT];
ComPtr<ID3D12DescriptorHeap> dsvheap;
ComPtr<ID3D12Resource> dsvs[FRAME_COUNT];

ComPtr<ID3D12DescriptorHeap> resheap;
ComPtr<ID3D12DescriptorHeap> imguiheap;

ComPtr<ID3D12Resource> iBuffer;
D3D12_INDEX_BUFFER_VIEW iBufferView;

ComPtr<ID3D12RootSignature> rootSigs;

// Shader Compiler objects
ComPtr<IDxcCompiler3> compiler;
ComPtr<IDxcUtils> utils;
ComPtr<IDxcIncludeHandler> inclHandler;

UINT frameIndex;
HANDLE fenceEvent;
ComPtr<ID3D12Fence> fence;
UINT64 fenceValues[FRAME_COUNT];

TracyD3D12Ctx ctx;

inline void ThrowIfFailed(HRESULT hr);
void GetHardwareAdapter(IDXGIFactory1 *pfactory, IDXGIAdapter1 **ppAdapter, bool hpAdapter);
void WaitForGPU();
void MoveToNextFrame();
HRESULT CompileShader(ComPtr<IDxcBlobEncoding> &src, LPCWSTR entry, LPCWSTR target, ComPtr<IDxcBlob> &shader);

void Setup(HWND hwnd, int width, int height) {
    printf("[RENDERER] Preparing renderer.\n");
    rtvDescSize = 0;
    dsvDescSize= 0;
    resDescSize = 0;
    frameIndex = 0;
    memset(fenceValues, 0, FRAME_COUNT * sizeof(UINT64));

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

    for (UINT i = 0; i < FRAME_COUNT; i++) {
        ThrowIfFailed(device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&cmdAllocs[i])
        ));

        ThrowIfFailed(device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmdAllocs[i].Get(),
            nullptr,
            IID_PPV_ARGS(&cmdLists[i])
        ));
        ThrowIfFailed(cmdLists[i]->Close());
    }
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

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc {};
        rtvHeapDesc.NumDescriptors = FRAME_COUNT;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvheap)));
        dsvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        D3D12_DESCRIPTOR_HEAP_DESC resHeapDesc {};
        resHeapDesc.NumDescriptors = 1;
        resHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        resHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(device->CreateDescriptorHeap(&resHeapDesc, IID_PPV_ARGS(&resheap)));
        resDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        D3D12_DESCRIPTOR_HEAP_DESC imguiHeapDesc {};
        imguiHeapDesc.NumDescriptors = 1;
        imguiHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        imguiHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(device->CreateDescriptorHeap(&imguiHeapDesc, IID_PPV_ARGS(&imguiheap)));
    }
    //-------------------------
    { // Frame Resources
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvhandle(rtvheap->GetCPUDescriptorHandleForHeapStart());
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvhandle(dsvheap->GetCPUDescriptorHandleForHeapStart());

        for (UINT i = 0; i < FRAME_COUNT; i++) {
            ThrowIfFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&rtvs[i])));
            device->CreateRenderTargetView(rtvs[i].Get(), nullptr, rtvhandle);
            rtvhandle.Offset(1, rtvDescSize);

            //TODO: Create depth stencil resource, and place view descriptor in heap
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
    { // Preparing shader compiler objects
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
        ThrowIfFailed(utils->CreateDefaultIncludeHandler(&inclHandler));
    }
    ctx = TracyD3D12Context(device.Get(), queue.Get());
    UI::initRender(device.Get(), FRAME_COUNT, DXGI_FORMAT_R8G8B8A8_UNORM, imguiheap.Get());
}

void Teardown() {
    printf("[RENDERER] Teardown renderer.\n");
    WaitForGPU();

    TracyD3D12Destroy(ctx);
    CloseHandle(fenceEvent);
}

int CreateRootSignature(std::span<RootSigParam> params, std::span<RootSigSample> samplers) {
    D3D12_ROOT_PARAMETER rootparams[32]; //TODO: Look into this as a max value

    for (int i = 0; i < params.size(); i++) {
        CD3DX12_ROOT_PARAMETER p;
        if (params[i].descriptorType == RootSigParam::SRVDescriptor) {
            p.InitAsShaderResourceView(params[i].descriptorIndex);
        }
        else if (params[i].descriptorType == RootSigParam::CBVDescriptor) {
            p.InitAsConstantBufferView(params[i].descriptorIndex);
        }
        else if (params[i].descriptorType == RootSigParam::UAVDescriptor) {
            p.InitAsUnorderedAccessView(params[i].descriptorIndex);
        }
        rootparams[i] = p;
    }

    //TODO: Use this table to store vertex data instead of using HW vertex fetching?
    CD3DX12_ROOT_PARAMETER table;
    CD3DX12_DESCRIPTOR_RANGE range;
    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    table.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);
    rootparams[1] = table;

    CD3DX12_ROOT_SIGNATURE_DESC rootdesc {};
    rootdesc.Init((UINT)params.size()+1, rootparams, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> sign;
    ComPtr<ID3DBlob> error;
    ComPtr<ID3D12RootSignature> root;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootdesc, D3D_ROOT_SIGNATURE_VERSION_1, &sign, &error));
    ThrowIfFailed(device->CreateRootSignature(
        0,
        sign->GetBufferPointer(),
        sign->GetBufferSize(),
        IID_PPV_ARGS(&root)
    ));

    return 0;
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
    TracyD3D12NewFrame(ctx);

    const UINT64 currentFenceValue = fenceValues[frameIndex];
    ThrowIfFailed(queue->Signal(fence.Get(), currentFenceValue));

    frameIndex = swapchain->GetCurrentBackBufferIndex();

    if (fence->GetCompletedValue() < fenceValues[frameIndex]) {
        ThrowIfFailed(fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent));
        WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
    }

    fenceValues[frameIndex] = currentFenceValue + 1;
}

HRESULT CompileShader(ComPtr<IDxcBlobEncoding> &src, LPCWSTR entry, LPCWSTR target, ComPtr<IDxcBlob> &shader) {
    printf("[SHADER] Compiling: entry = '%ls'; target = '%ls'\n", entry, target);
    const UINT32 argCount = 4;
    LPCWSTR extras[argCount] {
        DXC_ARG_PACK_MATRIX_ROW_MAJOR,
        DXC_ARG_WARNINGS_ARE_ERRORS,
        DXC_ARG_ALL_RESOURCES_BOUND,
        DXC_ARG_DEBUG
        //TODO: Look into other arguments
    };
    ComPtr<IDxcCompilerArgs> args;
    ThrowIfFailed(utils->BuildArguments(L"?", entry, target, extras, argCount, nullptr, 0, &args));

    DxcBuffer buffer { .Ptr = src->GetBufferPointer(), .Size = src->GetBufferSize(), .Encoding = 0u };
    ComPtr<IDxcResult> out{};
    ThrowIfFailed(compiler->Compile(
        &buffer,
        args->GetArguments(), args->GetCount(),
        inclHandler.Get(), IID_PPV_ARGS(&out)
    ));

    HRESULT hr;
    out->GetStatus(&hr);
    if (FAILED(hr)) {
        ComPtr<IDxcBlobEncoding> err;
        out->GetErrorBuffer(&err);

        printf("[SHADER] Could not compile %ls : %ls\n", entry, target);
        printf("error: %s\n", (char*)err->GetBufferPointer());
        ThrowIfFailed(hr);
    }
    else {
        out->GetResult(&shader);
    }
    return hr;
}

}
