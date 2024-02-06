#include "renderer.h"

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <d3d12shader.h>

#include <cassert>
#include <wrl.h>

#include "imgui_impl_dx12.h"
#include "TracyD3D12.hpp"

#define FRAME_COUNT 2

namespace Render {
using namespace Microsoft::WRL;
using namespace DirectX;

CD3DX12_VIEWPORT viewport;
CD3DX12_RECT scissor;
UINT rtvDescSize;

// Dx12 objects
ComPtr<ID3D12Device> device;
ComPtr<ID3D12CommandQueue> queue;
ComPtr<ID3D12CommandAllocator> cmdAllocs[FRAME_COUNT];
ComPtr<ID3D12GraphicsCommandList> cmdLists[FRAME_COUNT];
ComPtr<IDXGISwapChain4> swapchain;
ComPtr<ID3D12DescriptorHeap> rtvheap;
ComPtr<ID3D12DescriptorHeap> imguiheap;
ComPtr<ID3D12Resource> rtvs[FRAME_COUNT];

ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

struct Pipelines {
    std::vector<ComPtr<ID3D12RootSignature>> rootSigs;
    std::vector<ComPtr<ID3D12PipelineState>> PSOs;
} pipelines;

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

void setup(HWND hwnd, int width, int height) {
    printf("[RENDERER] Preparing renderer.\n");
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
}

void initImGui() {
    ImGui_ImplDX12_Init(device.Get(), FRAME_COUNT, DXGI_FORMAT_R8G8B8A8_UNORM,
        imguiheap.Get(),
        imguiheap->GetCPUDescriptorHandleForHeapStart(),
        imguiheap->GetGPUDescriptorHandleForHeapStart()
    );
}

void StartFrame() {
    ImGui_ImplDX12_NewFrame();

    ThrowIfFailed(cmdAllocs[frameIndex]->Reset());
    ID3D12GraphicsCommandList *cmdlist = cmdLists[frameIndex].Get();
    ThrowIfFailed(cmdlist->Reset(cmdAllocs[frameIndex].Get(), nullptr));

    //TODO: Do we have to split this into separate function
    cmdlist->RSSetViewports(1, &viewport);
    cmdlist->RSSetScissorRects(1, &scissor);

    D3D12_RESOURCE_BARRIER targetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(rtvs[frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    cmdlist->ResourceBarrier(1, &targetBarrier);

    //TODO: Handle giving a custom render target? Or split into function
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvheap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescSize);
    cmdlist->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
    const float clear[] { 0.f, 0.2f, 0.4f, 1.f };
    cmdlist->ClearRenderTargetView(rtvHandle, clear, 0, nullptr);
}

void EndFrame() {
    //TODO: Is this logic at the right spot?
    ID3D12DescriptorHeap *heaps { imguiheap.Get() };
    cmdLists[frameIndex]->SetDescriptorHeaps(1, &heaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdLists[frameIndex].Get());

    D3D12_RESOURCE_BARRIER presentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(rtvs[frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
    );
    cmdLists[frameIndex]->ResourceBarrier(1, &presentBarrier);

    ThrowIfFailed(cmdLists[frameIndex]->Close());
    ID3D12CommandList *ppCmdList[] { cmdLists[frameIndex].Get() };
    queue->ExecuteCommandLists(_countof(ppCmdList), ppCmdList);

    ThrowIfFailed(swapchain->Present(1, 0));
    MoveToNextFrame();
}

void teardown() {
    printf("[RENDERER] Teardown renderer.\n");
    WaitForGPU();

    TracyD3D12Destroy(ctx);
    ImGui_ImplDX12_Shutdown();
    CloseHandle(fenceEvent);
}

UINT64 CreateRootSignature(std::span<D3D12_ROOT_PARAMETER> params, std::span<D3D12_STATIC_SAMPLER_DESC> samplers) {
    CD3DX12_ROOT_SIGNATURE_DESC rootdesc {};
    rootdesc.Init((UINT)params.size(), params.data(), (UINT)samplers.size(), samplers.data(),
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

    UINT64 index = pipelines.rootSigs.size();
    pipelines.rootSigs.push_back(root);
    return index;
}

UINT64 CreatePSO(LPCWSTR shaderFile, LPCWSTR vertEntry, LPCWSTR pixEntry) {
    ComPtr<IDxcBlobEncoding> src{};
    ThrowIfFailed(utils->LoadFile(shaderFile, nullptr, &src));
    ComPtr<IDxcBlob> vs, ps;
    CompileShader(src, vertEntry, L"vs_6_5", vs);
    CompileShader(src, pixEntry, L"ps_6_5", ps);

    D3D12_INPUT_ELEMENT_DESC inputElem[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    ComPtr<ID3D12PipelineState> pso;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psodesc = { 0 };
    psodesc.InputLayout = { inputElem, _countof(inputElem) };
    psodesc.pRootSignature = pipelines.rootSigs[0].Get();
    psodesc.VS = { .pShaderBytecode = vs->GetBufferPointer(), .BytecodeLength = vs->GetBufferSize() };
    psodesc.PS = { .pShaderBytecode = ps->GetBufferPointer(), .BytecodeLength = ps->GetBufferSize() };
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

    UINT64 index = pipelines.PSOs.size();
    pipelines.PSOs.push_back(pso);
    return index;
}

void UploadVertexData(std::span<UploadData> uploadData, Draws &draws) {
    draws.startIndex.resize(uploadData.size());
    draws.vertCount.resize(uploadData.size());

    UINT64 num = 0;
    Vertex verts[1024]; //TODO: Look into if there's a better default value
    for (int i = 0; i < uploadData.size(); i++) {
        draws.startIndex[i] = num;
        draws.vertCount[i] = uploadData[i].verts.size();

        memcpy(&verts[num], uploadData[i].verts.data(), sizeof(Vertex) * draws.vertCount[i]);
        num += draws.vertCount[i];
    }

    const UINT vertBufferSize = sizeof(Vertex) * (UINT)num;
    D3D12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(vertBufferSize);
    ThrowIfFailed(device->CreateCommittedResource(
        &prop, D3D12_HEAP_FLAG_NONE,
        &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(&vertexBuffer)
    ));
    //TODO: Try to convert committed resources into placed resources for vertex buffers

    UINT8 *pVertDataBegin;
    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(vertexBuffer->Map(0, &readRange, (void**)&pVertDataBegin));
    memcpy(pVertDataBegin, verts, vertBufferSize);
    vertexBuffer->Unmap(0, nullptr);

    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = vertBufferSize;
    vertexBufferView.StrideInBytes = sizeof(Vertex);

    WaitForGPU();
}

void UseCamera(Camera &cam) {
    //TODO: Prepare and upload camera matrix
    // Bind camera data to be used for next draws

    //M: Rendered object won't move, so model matrix is XMMatrixIdentity
    //V: XMMatrixLookToLH(FXMVECTOR EyePosition, FXMVECTOR EyeDirection, FXMVECTOR UpDirection)
    //P: XMMatrixPerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
}

void RecordDraws(UINT64 rootSigIndex, UINT64 psoIndex, UINT64 startIndex, UINT64 vertexCount) {
    //TODO: require all parameters needed to prep:
    // PSO
    // ROOT SIG PARAMS
    // PUSH CONSTANTS
    // VERTEX OFFSET + COUNT
    // INDEX OFFSET + COUNT
    // Should be called from app
    ID3D12GraphicsCommandList *cmdlist = cmdLists[frameIndex].Get();

    cmdlist->SetGraphicsRootSignature(pipelines.rootSigs[rootSigIndex].Get());
    cmdlist->SetPipelineState(pipelines.PSOs[psoIndex].Get());

    cmdlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdlist->IASetVertexBuffers(0, 1, &vertexBufferView);
    cmdlist->DrawInstanced((UINT)vertexCount, 1, (UINT)startIndex, 0);
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
