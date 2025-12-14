
// Material/Materials
#include "M_SimpleLit.h"

// Scene
#include "SceneManager.h"

// System/Rendering
#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

void M_SimpleLit::Initialize(Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
		// Params
		constant.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // カメラ
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // ライト
	}

	RootSignature::Parameter param[] = 
	{
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::Description rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);

	Pipeline::Description pipeline;
	pipeline.VSFile = L"../exe/assets/shader/VS_Object.cso";
	pipeline.PSFile = L"../exe/assets/shader/PS_SimpleLit.cso";
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

void M_OpaqueSimpleLit::Initialize(Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
		// Params
		constant.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // カメラ
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // ライト
		constant.size = sizeof(CommonParam);
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
	}

	RootSignature::Parameter param[] = 
	{
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::Description rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);

	Pipeline::Description pipeline;
	pipeline.VSFile = L"../exe/assets/shader/VS_Object.cso";
	pipeline.PSFile = L"../exe/assets/shader/PS_OpaqueSimpleLit.cso";
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

	common.AlphaCut = 0.2f;
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
