
// Material/Materials
#include "M_SkyBox.h"

void M_SkyBox::Initialize(Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
	}

	RootSignature::Parameter param[] = 
	{
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::Description rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);

	Pipeline::Description pipeline;
	pipeline.VSFile = L"../game/assets/shader/VS_Object.cso";
	pipeline.PSFile = L"../game/assets/shader/PS_UnLit.cso";
	pipeline.pInputLayout = Pipeline::IED_POS_NOR_TEX_COLOR;
	pipeline.InputLayoutNum = Pipeline::IED_POS_NOR_TEX_COLOR_COUNT;
	pipeline.RenderTargetNum = 1;
	pipeline.CullMode = desc.CullMode;
	pipeline.WriteDepth = desc.WriteDepth;

	Material::SetUp(
		desc.pHeap,
		rootsignature,
		pipeline
	);
}

void M_SkyBox::Bind()
{
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Textures[0]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
