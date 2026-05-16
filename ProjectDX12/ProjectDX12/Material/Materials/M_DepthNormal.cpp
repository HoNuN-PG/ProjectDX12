
#include "Material/Materials/M_DepthNormal.h"
#include "System/Rendering/Pass/DepthNormalPass.h"
#include "System/Rendering/RenderingEngine.h"

void M_DepthNormal::Initialize(Description desc)
{
	// ルートシグネチャ
	RootSignature::Parameter param[] = 
	{
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
	};
	RootSignature::Description rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);

	// パイプライン
	std::vector<DXGI_FORMAT> formats = { GetRenderingEngine().lock()->GetPassFormat(Timing,PassType) };
	PipelineState::Description pipeline = {
		L"",
		L"",
		L"../game/assets/shader/VS_DepthNormal.cso",
		L"../game/assets/shader/PS_DepthNormal.cso",
		nullptr,
		PipelineState::IED_POS_NOR_TEX_COLOR,
		PipelineState::IED_POS_NOR_TEX_COLOR_COUNT,
		desc.CullMode,
		2,
		formats,
		desc.WriteDepth,
	};

	Material::SetUp(
		desc.pHeap,
		rootsignature,
		pipeline
	);
}

void M_DepthNormal::Bind(UINT materialinstance)
{
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[materialinstance]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
