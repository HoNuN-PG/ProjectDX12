
#include "M_Shadow.h"

#include "SceneManager.h"

#include "GameObject.h"

#include "ShadowPass.h"

#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

UINT M_ShadowMapsBase::CurrentShadowMapsNo = 0;

void M_SimpleShadowMaps::Initialize(DescriptorHeap* heap)
{
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// Params
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // ライト
		desc.size = sizeof(ShadowParam::ShadowMapsParam);
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // シャドウマップ1
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // シャドウマップ2
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // シャドウマップ3
	}

	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_VERTEX},
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
	pipeline.VSFile = L"../exe/assets/shader/VS_ShadowMap.cso";
	pipeline.PSFile = L"../exe/assets/shader/PS_SimpleShadowMap.cso";
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

void M_SimpleShadowMaps::Bind()
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();

	// 定数バッファの設定
	WriteParams((UINT)1, 0,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Light).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	WriteParams((UINT)1, 1 + CurrentShadowMapsNo,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowMaps1 + CurrentShadowMapsNo).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1 + CurrentShadowMapsNo]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}

void M_ShadowRecieverBase::Initialize(DescriptorHeap* heap)
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	std::weak_ptr<ShadowPass> pass = engine.lock()->GetShadowMapsPass();

	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// Params
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
		desc.size = sizeof(ShadowParam::ShadowReceieverParam);
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
	}
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.format = pass.lock()->ShadowMapsFormat;
		desc.pRTVHeap = RTVHeap.get();
		desc.pSRVHeap = heap;

		for (int i = 0; i < ShadowPass::TextureType::MAX; ++i)
		{
			desc.width = pass.lock()->ShadowMapsSize[i].x;
			desc.height = pass.lock()->ShadowMapsSize[i].y;
			ShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
		}
	}
}

void M_ShadowRecieverBase::Bind()
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();

	// 定数バッファの設定
	WriteParams((UINT)2, 0,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	WriteParams((UINT)1, 2,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowReciever).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// テクスチャコピー
	std::weak_ptr<RenderingEngine> Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Near].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::Near);
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Middle].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::Middle);
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Far].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::Far);
}

void M_ShadowVSMRecieverBase::Initialize(DescriptorHeap* heap)
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	std::weak_ptr<ShadowPass> pass = engine.lock()->GetShadowMapsPass();

	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// Params
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
		desc.size = sizeof(ShadowParam::ShadowReceieverParam);
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
	}
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.format = pass.lock()->ShadowMapsFormat;
		desc.pRTVHeap = RTVHeap.get();
		desc.pSRVHeap = heap;

		for (int i = 0; i < ShadowPass::TextureType::MAX; ++i)
		{
			desc.width = pass.lock()->VSMShadowMapsSize[i].x / 2;
			desc.height = pass.lock()->VSMShadowMapsSize[i].y / 2;
			ShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
		}
	}
}

void M_ShadowVSMRecieverBase::Bind()
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();

	// 定数バッファの設定
	WriteParams((UINT)2, 0,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	WriteParams((UINT)1, 2,
		engine.lock()->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowReciever).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// テクスチャコピー
	std::weak_ptr<RenderingEngine> Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Near].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::NearVSM);
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Middle].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::MiddleVSM);
	Engine.lock()->CopyPassTextureSRV(ShadowMaps[ShadowPass::Far].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::FarVSM);
}
