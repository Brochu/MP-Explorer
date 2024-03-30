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
#define DESC_TABLE_SIZE 256
#define ThrowIfFailed(hr)   \
do {                        \
    if (FAILED(hr)) {       \
        assert(false);      \
    }                       \
} while(0);                 \

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
ComPtr<ID3D12Resource> dsv;

ComPtr<ID3D12DescriptorHeap> resheap;
ComPtr<ID3D12DescriptorHeap> imguiheap;

ComPtr<ID3D12Resource> vBuffer[FRAME_COUNT];
ComPtr<ID3D12Resource> iBuffer;
D3D12_INDEX_BUFFER_VIEW iBufferView;

ComPtr<ID3D12RootSignature> rootSig;
struct PsoArray {
    ComPtr<ID3D12PipelineState> objs[32];
    size_t count;
} PSOs;

// Shader Compiler objects
ComPtr<IDxcCompiler3> compiler;
ComPtr<IDxcUtils> utils;
ComPtr<IDxcIncludeHandler> inclHandler;

UINT frameIndex;
HANDLE fenceEvent;
ComPtr<ID3D12Fence> fence;
UINT64 fenceValues[FRAME_COUNT];

TracyD3D12Ctx ctx;

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
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
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

        for (UINT i = 0; i < FRAME_COUNT; i++) {
            ThrowIfFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&rtvs[i])));
            device->CreateRenderTargetView(rtvs[i].Get(), nullptr, rtvhandle);
            rtvhandle.Offset(1, rtvDescSize);
        }

        D3D12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        D3D12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        D3D12_CLEAR_VALUE depthClear { DXGI_FORMAT_D32_FLOAT, { {1.f, 0} } };

        ThrowIfFailed(device->CreateCommittedResource(
            &prop, D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthClear, IID_PPV_ARGS(&dsv)
        ));

        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvhandle(dsvheap->GetCPUDescriptorHandleForHeapStart());
        device->CreateDepthStencilView(dsv.Get(), nullptr, dsvhandle);
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

void CreateRootSignature(std::span<RootSigParam> params, std::span<RootSigSample> samplers) {
    D3D12_STATIC_SAMPLER_DESC samplerDesc[32];
    for (int i = 0; i < samplers.size(); i++) {
        samplerDesc[i] = D3D12_STATIC_SAMPLER_DESC {};
        //TODO: Fill in sampler data
    }

    D3D12_ROOT_PARAMETER1 paramDesc[32];
    for (int i = 1; i <= params.size(); i++) {
        CD3DX12_ROOT_PARAMETER1 p;
        RootSigParam param = params[i - 1];

        if (param.descriptorType == RootSigParam::SRVDescriptor) {
            p.InitAsShaderResourceView(param.descriptorIndex);
        }
        else if (param.descriptorType == RootSigParam::CBVDescriptor) {
            p.InitAsConstantBufferView(param.descriptorIndex);
        }
        else if (param.descriptorType == RootSigParam::UAVDescriptor) {
            p.InitAsUnorderedAccessView(param.descriptorIndex);
        }
        p.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; //TODO: This is most likely wrong
        paramDesc[i] = p;
    }

    // Bindless table
    D3D12_DESCRIPTOR_RANGE1 bufferRange[1];
    bufferRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    bufferRange[0].NumDescriptors = DESC_TABLE_SIZE;
    bufferRange[0].BaseShaderRegister = 0;
    bufferRange[0].RegisterSpace = 0;
    bufferRange[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
    bufferRange[0].OffsetInDescriptorsFromTableStart = 0;

    paramDesc[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    paramDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    paramDesc[0].DescriptorTable = { 1, bufferRange };

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc {};
    rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSigDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    rootSigDesc.Desc_1_1.NumStaticSamplers = (UINT)samplers.size();
    rootSigDesc.Desc_1_1.pStaticSamplers = samplerDesc;
    rootSigDesc.Desc_1_1.NumParameters = (UINT)params.size() + 1;
    rootSigDesc.Desc_1_1.pParameters = paramDesc;

    ID3DBlob *sig;
    ID3DBlob *err;
    ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rootSigDesc, &sig, &err));
    ThrowIfFailed(device->CreateRootSignature(
        0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&rootSig))
    );
}

size_t CreatePSO(LPCWSTR path) {
    ComPtr<IDxcBlobEncoding> file;
    ThrowIfFailed(utils->LoadFile(path, 0, &file));

    ComPtr<IDxcBlob> vs;
    ComPtr<IDxcBlob> ps;
    ThrowIfFailed(CompileShader(file, L"VSMain", L"vs_6_5", vs));
    ThrowIfFailed(CompileShader(file, L"PSMain", L"ps_6_5", ps));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc { 0 };
    psoDesc.InputLayout = { 0 };
    psoDesc.pRootSignature = rootSig.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
    psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    //TODO: See if I can send params from App to drive parameters here

    ThrowIfFailed(device->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&PSOs.objs[PSOs.count])
    ));
    return PSOs.count++;
}

size_t UploadVertBuffer(void *data, size_t size, size_t stride) {
    //TODO: Upload vertex buffer to be bound in main root signature table
    return 0;
}

size_t UploadIndexBuffer(void *data, size_t size, size_t stride) {
    //TODO: Upload index buffer to be bound to command buffer
    return 0;
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
