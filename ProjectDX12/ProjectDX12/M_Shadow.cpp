
// Material/Materials
#include "M_Shadow.h"

// Scene
#include "SceneManager.h"

// System/GameObject
#include "GameObject.h"
// System/Rendering/Pass
#include "ShadowPass.h"
// System/Rendering
#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

UINT M_ShadowMapsBase::CurrentShadowMapsNo = 0;

void M_SimpleShadowMaps::Initialize(Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
		// Params
		constant.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // ライト
		constant.size = sizeof(ShadowParam::ShadowMapsParam);
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // シャドウマップ1
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // シャドウマップ2
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // シャドウマップ3
	}

	// ルートシグネチャ
	RootSignature::Parameter param[] = 
	{
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_VERTEX},
	};
	RootSignature::Description rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);
	rootsignature.sample = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	// パイプライン
	Pipeline::Description pipeline;
	pipeline.VSFile = L"../game/assets/shader/VS_ShadowMap.cso";
	pipeline.PSFile = L"../game/assets/shader/PS_SimpleShadowMap.cso";
	pipeline.pInputLayout = Pipeline::IED_POS_NOR_TEX_COLOR;
	pipeline.InputLayoutNum = Pipeline::IED_POS_NOR_TEX_COLOR_COUNT;
	pipeline.CullMode = desc.CullMode;
	pipeline.RenderTargetNum = 1;
	pipeline.WriteDepth = desc.WriteDepth;

	Material::SetUp(
		desc.pHeap,
		rootsignature,
		pipeline
	);
}

void M_SimpleShadowMaps::Bind(UINT materialinstance)
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();

	// 定数バッファの設定
	WriteParams(
		(UINT)1, 
		0,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Light).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	WriteParams(
		(UINT)1, 
		1 + CurrentShadowMapsNo,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowMaps1 + CurrentShadowMapsNo).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[materialinstance]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1 + CurrentShadowMapsNo]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}

void M_OpaqueSimpleShadowMaps::Initialize(Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
		// Params
		constant.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // ライト
		constant.size = sizeof(ShadowParam::ShadowMapsParam);
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // シャドウマップ1
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // シャドウマップ2
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // シャドウマップ3
		constant.size = sizeof(CommonParam);
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
	}

	RootSignature::Parameter param[] = 
	{
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::Description rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);
	rootsignature.sample = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	Pipeline::Description pipeline;
	pipeline.VSFile = L"../game/assets/shader/VS_ShadowMap.cso";
	pipeline.PSFile = L"../game/assets/shader/PS_OpaqueSimpleShadowMap.cso";
	pipeline.pInputLayout = Pipeline::IED_POS_NOR_TEX_COLOR;
	pipeline.InputLayoutNum = Pipeline::IED_POS_NOR_TEX_COLOR_COUNT;
	pipeline.CullMode = desc.CullMode;
	pipeline.RenderTargetNum = 1;
	pipeline.WriteDepth = desc.WriteDepth;

	Material::SetUp(
		desc.pHeap,
		rootsignature,
		pipeline
	);

	common.AlphaCut = 0.2f;
}

void M_OpaqueSimpleShadowMaps::Bind(UINT materialinstance)
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();

	// 定数バッファの設定
	WriteParams(
		(UINT)1, 
		0,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Light).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	WriteParams(
		(UINT)1, 
		1 + CurrentShadowMapsNo,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowMaps1 + CurrentShadowMapsNo).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	Params[1 + SHADOW_MAP_COUNT]->Write(&common);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[materialinstance]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1 + CurrentShadowMapsNo]->GetHandle().hGPU,
		Textures[0]->GetHandle().hGPU,
		Params[1 + SHADOW_MAP_COUNT]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}

void M_ShadowRecieverBase::Initialize(Description desc)
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	std::weak_ptr<ShadowPass> pass = engine.lock()->GetShadowMapsPass();

	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
		// Params
		constant.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
		constant.size = sizeof(ShadowParam::ShadowReceieverParam);
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
	}
	// RTV
	{
		RenderTarget::Description rtv = {};
		rtv.format = pass.lock()->ShadowMapsFormat;
		rtv.pRTVHeap = RTVHeap.get();
		rtv.pSRVHeap = desc.pHeap;
		for (int i = 0; i < ShadowPass::TextureType::MAX; ++i)
		{
			rtv.width = pass.lock()->ShadowMapsSize[i].x;
			rtv.height = pass.lock()->ShadowMapsSize[i].y;
			ShadowMaps.push_back(std::make_shared<RenderTarget>(rtv));
		}
	}
}

void M_ShadowRecieverBase::Bind(UINT materialinstance)
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();

	// 定数バッファの設定
	WriteParams(
		(UINT)2, 
		0,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	WriteParams(
		(UINT)1, 
		2,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowReciever).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);

	// テクスチャコピー
	std::weak_ptr<RenderingEngine> Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Near].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::Near);
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Middle].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::Middle);
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Far].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::Far);
}

void M_ShadowVSMRecieverBase::Initialize(Description desc)
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	std::weak_ptr<ShadowPass> pass = engine.lock()->GetShadowMapsPass();

	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
		// Params
		constant.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
		constant.size = sizeof(ShadowParam::ShadowReceieverParam);
		Params.push_back(std::make_unique<ConstantBuffer>(constant));
	}
	// RTV
	{
		RenderTarget::Description rtv = {};
		rtv.format = pass.lock()->ShadowMapsFormat;
		rtv.pRTVHeap = RTVHeap.get();
		rtv.pSRVHeap = desc.pHeap;
		for (int i = 0; i < ShadowPass::TextureType::MAX; ++i)
		{
			rtv.width = pass.lock()->VSMShadowMapsSize[i].x / 2;
			rtv.height = pass.lock()->VSMShadowMapsSize[i].y / 2;
			ShadowMaps.push_back(std::make_shared<RenderTarget>(rtv));
		}
	}
}

void M_ShadowVSMRecieverBase::Bind(UINT materialinstance)
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();

	// 定数バッファの設定
	WriteParams(
		(UINT)2, 
		0,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	WriteParams(
		(UINT)1, 
		2,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowReciever).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);

	// テクスチャコピー
	std::weak_ptr<RenderingEngine> Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Near].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::NearVSM);
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Middle].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::MiddleVSM);
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Far].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::FarVSM);
}
