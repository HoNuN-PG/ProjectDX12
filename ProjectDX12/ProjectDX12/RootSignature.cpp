
#include "RootSignature.h"

#include <vector>

RootSignature::RootSignature(Description desc)
{
	// ルートシグネチャの生成
	// ルートパラメータの設定
	std::vector<D3D12_ROOT_PARAMETER> param;
	std::vector<D3D12_DESCRIPTOR_RANGE> range;
	param.resize(desc.paramNum);
	range.resize(desc.paramNum);
	for (UINT i = 0; i < desc.paramNum; ++i)
	{
		range[i].RangeType								= desc.pParam[i].type;
		range[i].BaseShaderRegister						= desc.pParam[i].slot;
		range[i].NumDescriptors							= desc.pParam[i].num;
		range[i].OffsetInDescriptorsFromTableStart		= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		param[i].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[i].DescriptorTable.NumDescriptorRanges	= 1;
		param[i].DescriptorTable.pDescriptorRanges		= &range[i];
		param[i].ShaderVisibility						= desc.pParam[i].shader;
	}

	// サンプラ
	D3D12_STATIC_SAMPLER_DESC sampler	= {};
	sampler.Filter						= D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ComparisonFunc				= D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor					= D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	sampler.MinLOD						= 0.0f;
	sampler.MaxLOD						= D3D12_FLOAT32_MAX;
	sampler.ShaderVisibility			= D3D12_SHADER_VISIBILITY_PIXEL;

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC signatureDesc = {};
	signatureDesc.NumParameters				= desc.paramNum;
	signatureDesc.pParameters				= param.data();;
	signatureDesc.NumStaticSamplers			= 1;
	signatureDesc.pStaticSamplers			= &sampler;
	signatureDesc.Flags						= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// バイナリコード生成
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signatureBlob, &errorBlob
	);
	if (FAILED(hr)) { return; }

	// ルートシグネチャ生成
	hr = GetDevice()->CreateRootSignature(
		0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature)
	);
}

RootSignature::~RootSignature()
{
	m_pRootSignature->Release();
}

void RootSignature::Bind(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT num)
{
	GetCommandList()->SetGraphicsRootSignature(m_pRootSignature);
	for (int i = 0; i < num; ++i)
		GetCommandList()->SetGraphicsRootDescriptorTable(i, handle[i]);
}
