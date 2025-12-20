
// Material/Materials
#include "M_DepthNormal.h"

void M_DepthNormal::Initialize(Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
	}

	// ルートシグネチャ
	RootSignature::Parameter param[] = 
	{
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
	};
	RootSignature::Description rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);

	// パイプライン
	Pipeline::Description pipeline;
	pipeline.VSFile = L"../game/assets/shader/VS_DepthNormal.cso";
	pipeline.PSFile = L"../game/assets/shader/PS_DepthNormal.cso";
	pipeline.pInputLayout = Pipeline::IED_POS_NOR_TEX_COLOR;
	pipeline.InputLayoutNum = Pipeline::IED_POS_NOR_TEX_COLOR_COUNT;
	pipeline.RenderTargetNum = 2;
	pipeline.CullMode = desc.CullMode;
	pipeline.WriteDepth = desc.WriteDepth;

	Material::SetUp(
		desc.pHeap,
		rootsignature,
		pipeline
	);
}

void M_DepthNormal::Bind()
{
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
