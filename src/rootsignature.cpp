#include "rootsignature.h"
#include <d3d12.h>

namespace RootSignature {
using namespace Microsoft::WRL;

std::vector<CD3DX12_ROOT_PARAMETER1> paramArray;
std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplerArray;

ComPtr<ID3D12RootSignature> rootSig;

CD3DX12_ROOT_PARAMETER1 &ParamAt(size_t idx) {
    //TODO: Define ASSERT macro
    return paramArray[idx];
}
CD3DX12_STATIC_SAMPLER_DESC &SamplerAt(size_t idx) {
    return samplerArray[idx];
}

void Init(size_t numParams, size_t numSamplers) {
    paramArray.resize(numParams);
    samplerArray.resize(numSamplers);
}

void Compile() {
}

ID3D12RootSignature *Get() {
    return rootSig.Get();
}

}
