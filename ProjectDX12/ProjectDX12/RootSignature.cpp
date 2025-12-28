
// System/Rendering/Pipeline
#include "RootSignature.h"

RootSignature::RootSignature(Description desc)
{
	// ルートシグネチャの生成
	std::vector<D3D12_DESCRIPTOR_RANGE> range;
	std::vector<D3D12_ROOT_PARAMETER> param;
	range.resize(desc.paramNum);
	param.resize(desc.paramNum);
	BOOL bMeshShader = FALSE;
	for (UINT i = 0; i < desc.paramNum; ++i)
	{
		if(desc.pParam[i].type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV && desc.pParam[i].shader == D3D12_SHADER_VISIBILITY_MESH)
		{
			param[i].ParameterType						= D3D12_ROOT_PARAMETER_TYPE_SRV;
			param[i].Descriptor.ShaderRegister			= desc.pParam[i].slot;
			param[i].Descriptor.RegisterSpace			= 0;
			param[i].ShaderVisibility					= desc.pParam[i].shader;
		}
		else
		{
			range[i].RangeType								= desc.pParam[i].type;
			range[i].BaseShaderRegister						= desc.pParam[i].slot;						// シェーダー側のバインド開始インデックス(b0~など)
			range[i].NumDescriptors							= desc.pParam[i].num;
			range[i].OffsetInDescriptorsFromTableStart		= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			param[i].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			param[i].DescriptorTable.NumDescriptorRanges	= 1;										// レンジ数
			param[i].DescriptorTable.pDescriptorRanges		= &range[i];								// ディスクリプタがもつレンジの先頭アドレス
			param[i].ShaderVisibility						= desc.pParam[i].shader;
		}
	}

	SetUp(param, desc.sample, desc.filter, desc.paramNum, desc.bMeshShader);
}

RootSignature::~RootSignature()
{
}

void RootSignature::SetUp(std::vector<D3D12_ROOT_PARAMETER> param, D3D12_TEXTURE_ADDRESS_MODE sample, D3D12_FILTER filter, UINT num, BOOL bMeshShader)
{
	// サンプラ
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.AddressU			= sample;
	sampler.AddressV			= sample;
	sampler.AddressW			= sample;
	sampler.Filter				= filter;
	sampler.MaxAnisotropy		= 4;
	sampler.ComparisonFunc		= D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor			= D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	sampler.MinLOD				= 0.0f;
	sampler.MaxLOD				= D3D12_FLOAT32_MAX;
	sampler.ShaderVisibility	= D3D12_SHADER_VISIBILITY_PIXEL;

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC signatureDesc = {};
	signatureDesc.NumParameters		= num;
	signatureDesc.pParameters		= param.data();
	signatureDesc.NumStaticSamplers = 1;
	signatureDesc.pStaticSamplers	= &sampler;
	if(bMeshShader)
	{
		signatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	}
	else
	{
		signatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	}

	// バイナリコード生成
	ID3DBlob* signatureBlob		= nullptr;
	ID3DBlob* errorBlob			= nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&signatureDesc, 
		D3D_ROOT_SIGNATURE_VERSION_1_0, 
		&signatureBlob, 
		&errorBlob
	);
	if (FAILED(hr)) { return; }

	// ルートシグネチャ生成
	hr = GetDevice()->CreateRootSignature(
		0, 
		signatureBlob->GetBufferPointer(), 
		signatureBlob->GetBufferSize(), 
		IID_PPV_ARGS(&RootSignatureData)
	);
}

void RootSignature::Bind(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT num)
{
	GetCommandList()->SetGraphicsRootSignature(RootSignatureData.Get());
	for (int i = 0; i < num; ++i)
	{
		GetCommandList()->SetGraphicsRootDescriptorTable(i, handle[i]);
	}
}
