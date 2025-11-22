
#include "M_Deffered_Albedo_Normal.h"

void M_Deffered_Albedo_Normal::Initialize(DescriptorHeap* heap, Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
	}

	// ルートシグネチャ
	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::DescriptionTable rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);

	// パイプライン
	Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"NORMAL",   0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
			{"COLOR",    0,DXGI_FORMAT_R32G32B32A32_FLOAT},
	};
	Pipeline::Description pipeline;
	pipeline.cull = desc.cull;
	pipeline.WriteDepth = desc.WriteDepth;
	pipeline.VSFile = L"../exe/assets/shader/VS_Object.cso";
	pipeline.PSFile = L"../exe/assets/shader/PS_Deffered.cso";
	pipeline.pInputLayout = layout;
	pipeline.InputLayoutNum = _countof(layout);
	pipeline.RenderTargetNum = 2;

	Material::SetUp
	(
		heap,
		rootsignature,
		pipeline
	);
}

void M_Deffered_Albedo_Normal::Bind()
{
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Textures[0]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
