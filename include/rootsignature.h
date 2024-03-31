#pragma once
#include <d3dx12.h>

namespace RootSignature {

CD3DX12_ROOT_PARAMETER1 &ParamAt(size_t idx);
CD3DX12_STATIC_SAMPLER_DESC &SamplerAt(size_t idx);

void Init(size_t numParams, size_t numSamplers);
void Compile();

ID3D12RootSignature *Get();

}
