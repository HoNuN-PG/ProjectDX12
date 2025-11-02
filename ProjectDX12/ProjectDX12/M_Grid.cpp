
#include "M_Grid.h"

#include "SceneManager.h"

#include "GameObject.h"

#include "ShadowPass.h"

#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

void M_Grid::Initialize(DescriptorHeap* heap)
{
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		desc.size = sizeof(Grid::GridParam);
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
	}

	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::DescriptionTable rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);
	rootsignature.sample = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"NORMAL",   0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
			{"COLOR",    0,DXGI_FORMAT_R32G32B32A32_FLOAT},
	};
	Pipeline::Description pipeline;
	pipeline.cull = D3D12_CULL_MODE_BACK;
	pipeline.VSFile = L"../exe/assets/shader/VS_WorldObject.cso";
	pipeline.PSFile = L"../exe/assets/shader/PS_Grid.cso";
	pipeline.pInputLayout = layout;
	pipeline.InputLayoutNum = _countof(layout);
	pipeline.RenderTargetNum = 1;

	Material::SetUp
	(
		heap,
		rootsignature,
		pipeline
	);

	GridParam.GridWidth = 0.05f;
}

void M_Grid::Bind()
{
	// 定数バッファの設定
	WriteParams(&GridParam,0);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}

void M_GridShadow::Initialize(DescriptorHeap* heap)
{
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// Params
		desc.size = sizeof(Grid::GridParam);
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
	}

	RootSignature::ParameterTable param[] = {
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
	RootSignature::DescriptionTable rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);
	rootsignature.sample = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"NORMAL",   0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
			{"COLOR",    0,DXGI_FORMAT_R32G32B32A32_FLOAT},
	};
	Pipeline::Description pipeline;
	pipeline.cull = D3D12_CULL_MODE_BACK;
	pipeline.VSFile = L"../exe/assets/shader/VS_ShadowReciever.cso";
	pipeline.PSFile = L"../exe/assets/shader/PS_GridShadow.cso";
	pipeline.pInputLayout = layout;
	pipeline.InputLayoutNum = _countof(layout);
	pipeline.RenderTargetNum = 1;

	Material::SetUp
	(
		heap,
		rootsignature,
		pipeline,
		3
	);
	M_ShadowRecieverBase::Initialize(heap);

	GridParam.GridWidth = 0.05f;
}

void M_GridShadow::Bind()
{
	M_ShadowRecieverBase::Bind();

	// 定数バッファの設定
	WriteParams(&GridParam, 3);

	std::weak_ptr<RenderingEngine> Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
		Params[2]->GetHandle().hGPU,
		ShadowMaps[ShadowPass::Near]->GetHandleSRV().hGPU,
		ShadowMaps[ShadowPass::Middle]->GetHandleSRV().hGPU,
		ShadowMaps[ShadowPass::Far]->GetHandleSRV().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
		Params[2]->GetHandle().hGPU,
		Params[3]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}

void M_GridShadowVSM::Initialize(DescriptorHeap* heap)
{
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// Params
		desc.size = sizeof(Grid::GridParam);
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
	}

	RootSignature::ParameterTable param[] = {
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
	RootSignature::DescriptionTable rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);
	rootsignature.sample = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	rootsignature.filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"NORMAL",   0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
			{"COLOR",    0,DXGI_FORMAT_R32G32B32A32_FLOAT},
	};
	Pipeline::Description pipeline;
	pipeline.cull = D3D12_CULL_MODE_BACK;
	pipeline.VSFile = L"../exe/assets/shader/VS_ShadowReciever.cso";
	pipeline.PSFile = L"../exe/assets/shader/PS_GridShadowVSM.cso";
	pipeline.pInputLayout = layout;
	pipeline.InputLayoutNum = _countof(layout);
	pipeline.RenderTargetNum = 1;

	Material::SetUp
	(
		heap,
		rootsignature,
		pipeline,
		3
	);
	M_ShadowVSMRecieverBase::Initialize(heap);

	GridParam.GridWidth = 0.05f;
}

void M_GridShadowVSM::Bind()
{
	M_ShadowVSMRecieverBase::Bind();

	// 定数バッファの設定
	WriteParams(&GridParam, 3);

	std::weak_ptr<RenderingEngine> Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
		Params[2]->GetHandle().hGPU,
		ShadowMaps[ShadowPass::Near]->GetHandleSRV().hGPU,
		ShadowMaps[ShadowPass::Middle]->GetHandleSRV().hGPU,
		ShadowMaps[ShadowPass::Far]->GetHandleSRV().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
		Params[2]->GetHandle().hGPU,
		Params[3]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
