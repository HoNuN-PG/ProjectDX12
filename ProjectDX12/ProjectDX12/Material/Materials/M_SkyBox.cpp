
#include "Material/Materials/M_SkyBox.h"

void M_SkyBox::Initialize(Description desc)
{
	RootSignature::Parameter param[] = 
	{
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::Description rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);

	PipelineState::Description pipeline;
	pipeline.VSFile = L"../game/assets/shader/VS_Object.cso";
	pipeline.PSFile = L"../game/assets/shader/PS_UnLit.cso";
	pipeline.pInputLayout = PipelineState::IED_POS_NOR_TEX_COLOR;
	pipeline.InputLayoutNum = PipelineState::IED_POS_NOR_TEX_COLOR_COUNT;
	pipeline.CullMode = desc.CullMode;
	pipeline.RenderTargetNum = 1;
	pipeline.WriteDepth = desc.WriteDepth;

	Material::SetUp(
		desc.pHeap,
		rootsignature,
		pipeline
	);
}

void M_SkyBox::Bind(UINT materialinstance)
{
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[materialinstance]->GetHandle().hGPU,
		Textures[0]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
