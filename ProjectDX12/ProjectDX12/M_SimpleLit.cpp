
#include "M_SimpleLit.h"

#include "SceneManager.h"

#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

void M_SimpleLit::Initialize(DescriptorHeap* heap, Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// Params
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // カメラ
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // ライト
	}

	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::DescriptionTable rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);

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
	pipeline.PSFile = L"../exe/assets/shader/PS_SimpleLit.cso";
	pipeline.pInputLayout = layout;
	pipeline.InputLayoutNum = _countof(layout);
	pipeline.RenderTargetNum = 1;

	Material::SetUp
	(
		heap,
		rootsignature,
		pipeline
	);
}

void M_SimpleLit::Bind()
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();

	// 定数バッファの設定
	WriteParams((UINT)2, 0,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}

void M_OpaqueSimpleLit::Initialize(DescriptorHeap* heap, Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// Params
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // カメラ
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // ライト
		desc.size = sizeof(CommonParam);
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
	}

	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::DescriptionTable rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);

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
	pipeline.PSFile = L"../exe/assets/shader/PS_OpaqueSimpleLit.cso";
	pipeline.pInputLayout = layout;
	pipeline.InputLayoutNum = _countof(layout);
	pipeline.RenderTargetNum = 1;

	Material::SetUp
	(
		heap,
		rootsignature,
		pipeline
	);

	common.AlphaCut = 0.9f;
}

void M_OpaqueSimpleLit::Bind()
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();

	// 定数バッファの設定
	WriteParams((UINT)2, 0,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	Params[2]->Write(&common);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Textures[0]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
		Params[2]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
