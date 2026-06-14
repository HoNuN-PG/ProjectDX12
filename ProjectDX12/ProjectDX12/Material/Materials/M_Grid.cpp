
#include "Material/Materials/M_Grid.h"
#include "Scene/SceneManager.h"
#include "System/GameObject/GameObject.h"
#include "System/Rendering/Pass/ShadowPass.h"
#include "System/Rendering/GlobalResourceKey.h"
#include "System/Rendering/RenderingEngine.h"

void M_Grid::Initialize(Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
		constant.size = sizeof(Grid::GridParam);
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
	}

	// ルートシグネチャ
	RootSignature::Parameter param[] = 
	{
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::Description rootsignature =
	{
		param,
		_countof(param)
	};

	// パイプライン
	std::vector<DXGI_FORMAT> formats = { GetRenderingEngine().lock()->GetPassFormat(Timing,PassType) };
	PipelineState::Description pipeline = {
		L"",
		L"",
		L"../game/assets/shader/VS_WorldObject.cso",
		L"../game/assets/shader/PS_Grid.cso",
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

	GridParam.GridWidth = 0.05f;
}

void M_Grid::Bind(UINT materialinstance)
{
	// 定数バッファの設定
	WriteParam(&GridParam,0);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[materialinstance]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}

void M_GridShadow::Initialize(Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
		// Params
		constant.size = sizeof(Grid::GridParam);
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
	}

	// ルートシグネチャ
	RootSignature::Parameter param[] = 
	{
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 11, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 12, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::Description rootsignature =
	{
		param,
		_countof(param)
	};

	// パイプライン
	std::vector<DXGI_FORMAT> formats = { GetRenderingEngine().lock()->GetPassFormat(Timing,PassType) };
	PipelineState::Description pipeline = {
		L"",
		L"",
		L"../game/assets/shader/VS_ShadowReciever.cso",
		L"../game/assets/shader/PS_GridShadow.cso",
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
		pipeline,
		3
	);
	M_ShadowRecieverBase::Initialize(desc);

	GridParam.GridWidth = 0.05f;
}

void M_GridShadow::Bind(UINT materialinstance)
{
	M_ShadowRecieverBase::Bind(materialinstance);

	// 定数バッファの設定
	WriteParam(&GridParam, 3);

	std::weak_ptr<RenderingEngine> Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[materialinstance]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
		Params[2]->GetHandle().hGPU,
		ShadowMaps[ShadowPass::TextureType::Near]->GetHandleSRV().hGPU,
		ShadowMaps[ShadowPass::TextureType::Middle]->GetHandleSRV().hGPU,
		ShadowMaps[ShadowPass::TextureType::Far]->GetHandleSRV().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
		Params[2]->GetHandle().hGPU,
		Params[3]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}

void M_GridShadowVSM::Initialize(Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
		// Params
		constant.size = sizeof(Grid::GridParam);
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
	}

	// ルートシグネチャ
	RootSignature::Parameter param[] = 
	{
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 11, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 12, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::Description rootsignature =
	{
		param,
		_countof(param)
	};

	// パイプライン
	std::vector<DXGI_FORMAT> formats = { GetRenderingEngine().lock()->GetPassFormat(Timing,PassType) };
	PipelineState::Description pipeline = {
		L"",
		L"",
		L"../game/assets/shader/VS_ShadowReciever.cso",
		L"../game/assets/shader/PS_GridShadowVSM.cso",
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
		pipeline,
		3
	);
	M_ShadowVSMRecieverBase::Initialize(desc);

	GridParam.GridWidth = 0.05f;
}

void M_GridShadowVSM::Bind(UINT materialinstance)
{
	M_ShadowVSMRecieverBase::Bind(materialinstance);

	// 定数バッファの設定
	WriteParam(&GridParam, 3);

	std::weak_ptr<RenderingEngine> Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[materialinstance]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
		Params[2]->GetHandle().hGPU,
		ShadowMaps[ShadowPass::TextureType::Near]->GetHandleSRV().hGPU,
		ShadowMaps[ShadowPass::TextureType::Middle]->GetHandleSRV().hGPU,
		ShadowMaps[ShadowPass::TextureType::Far]->GetHandleSRV().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
		Params[2]->GetHandle().hGPU,
		Params[3]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
