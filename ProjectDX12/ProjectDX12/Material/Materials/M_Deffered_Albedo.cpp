
#include "Material/Materials/M_Deffered_Albedo.h"
#include "System/Rendering/RenderingEngine.h"

void M_Deffered_Albedo::Initialize(Description desc)
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
	std::vector<DXGI_FORMAT> formats = { GetRenderingEngine().lock()->GetPassFormat(Timing,PassType) };
	PipelineState::Description pipeline = {
		L"",
		L"",
		L"../game/assets/shader/VS_Object.cso",
		L"../game/assets/shader/PS_Deffered.cso",
		nullptr,
		PipelineState::IED_POS_NOR_TEX_COLOR,
		PipelineState::IED_POS_NOR_TEX_COLOR_COUNT,
		desc.CullMode,
		1,
		formats,
		desc.WriteDepth,
	};

	Material::SetUp(
		desc.pHeap,
		rootsignature,
		pipeline
	);
}

void M_Deffered_Albedo::Bind(UINT materialinstance)
{
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[materialinstance]->GetHandle().hGPU,
		Textures[0]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
