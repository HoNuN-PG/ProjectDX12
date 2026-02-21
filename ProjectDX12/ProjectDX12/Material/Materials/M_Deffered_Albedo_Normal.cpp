
#include "Material/Materials/M_Deffered_Albedo_Normal.h"

void M_Deffered_Albedo_Normal::Initialize(Description desc)
{
	// ルートシグネチャ
	RootSignature::Parameter param[] = 
	{
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::Description rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);

	// パイプライン
	PipelineState::Description pipeline;
	pipeline.VSFile = L"../game/assets/shader/VS_Object.cso";
	pipeline.PSFile = L"../game/assets/shader/PS_Deffered.cso";
	pipeline.pInputLayout = PipelineState::IED_POS_NOR_TEX_COLOR;
	pipeline.InputLayoutNum = PipelineState::IED_POS_NOR_TEX_COLOR_COUNT;
	pipeline.CullMode = desc.CullMode;
	pipeline.RenderTargetNum = 2;
	pipeline.WriteDepth = desc.WriteDepth;

	Material::SetUp(
		desc.pHeap,
		rootsignature,
		pipeline
	);
}

void M_Deffered_Albedo_Normal::Bind(UINT materialinstance)
{
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[materialinstance]->GetHandle().hGPU,
		Textures[0]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
