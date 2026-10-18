
#include "Material/Materials/M_Shadow.h"
#include "Scene/SceneManager.h"
#include "System/GameObject/GameObject.h"
#include "System/Rendering/Pass/ShadowPass.h"
#include "System/Rendering/GlobalResourceKey.h"
#include "System/Rendering/RenderingEngine.h"

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
	RootSignature::Description rootsignature =
	{
		param,
		_countof(param),
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	};

	// パイプライン
	std::vector<DXGI_FORMAT> formats = { GetRenderingEngine()->GetPassFormat(Timing,PassType) };
	PipelineState::Description pipeline = {
		L"",
		L"",
		L"../game/assets/shader/VS_ShadowMap.cso",
		L"../game/assets/shader/PS_SimpleShadowMap.cso",
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

void M_SimpleShadowMaps::Bind(UINT materialinstance)
{
	RenderingEngine* engine = SceneManager::GetRenderingEngine();
	assert(engine);

	// 定数バッファの設定
	WriteParams(
		(UINT)1, 
		0,
		engine->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Light).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	WriteParams(
		(UINT)1, 
		1 + CurrentShadowMapsNo,
		engine->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowMaps1 + CurrentShadowMapsNo).hCPU, 
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
	RootSignature::Description rootsignature =
	{
		param,
		_countof(param),
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	};

	// パイプライン
	std::vector<DXGI_FORMAT> formats = { GetRenderingEngine()->GetPassFormat(Timing,PassType) };
	PipelineState::Description pipeline = {
		L"",
		L"",
		L"../game/assets/shader/VS_ShadowMap.cso",
		L"../game/assets/shader/PS_OpaqueSimpleShadowMap.cso",
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

	common.AlphaCut = 0.2f;
}

void M_OpaqueSimpleShadowMaps::Bind(UINT materialinstance)
{
	RenderingEngine* engine = SceneManager::GetRenderingEngine();
	assert(engine);

	// 定数バッファの設定
	WriteParams(
		(UINT)1, 
		0,
		engine->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Light).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	WriteParams(
		(UINT)1, 
		1 + CurrentShadowMapsNo,
		engine->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowMaps1 + CurrentShadowMapsNo).hCPU, 
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
	RenderingEngine* engine = SceneManager::GetRenderingEngine();
	assert(engine);
	ShadowPass* pass = engine->GetShadowMapsPass();
	assert(pass);

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
		rtv.format = pass->GetPassFormat()[0];
		rtv.pRTVHeap = pRTVHeap.get();
		rtv.pSRVHeap = desc.pHeap;
		for (int i = 0; i <= ShadowPass::TextureType::Far; ++i)
		{
			rtv.width = pass->ShadowMapsSize[i].x;
			rtv.height = pass->ShadowMapsSize[i].y;
			ShadowMaps.push_back(std::make_shared<RenderTarget>(rtv));
		}
	}
}

void M_ShadowRecieverBase::Bind(UINT materialinstance)
{
	RenderingEngine* engine = GetRenderingEngine();
	assert(engine);

	// 定数バッファの設定
	WriteParams(
		(UINT)2, 
		0,
		engine->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	WriteParams(
		(UINT)1, 
		2,
		engine->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowReciever).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);

	// テクスチャコピー
	engine->CopyPassTextureSRV(ShadowMaps[ShadowPass::Near].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::Near);
	engine->CopyPassTextureSRV(ShadowMaps[ShadowPass::Middle].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::Middle);
	engine->CopyPassTextureSRV(ShadowMaps[ShadowPass::Far].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::Far);
}

void M_ShadowVSMRecieverBase::Initialize(Description desc)
{
	RenderingEngine* engine = GetRenderingEngine();
	assert(engine);
	ShadowPass* pass = engine->GetShadowMapsPass();
	assert(pass);

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
		rtv.format = pass->GetPassFormat()[0];
		rtv.pRTVHeap = pRTVHeap.get();
		rtv.pSRVHeap = desc.pHeap;
		for (int i = 0; i <= ShadowPass::TextureType::Far; ++i)
		{
			rtv.width = pass->VSMShadowMapsSize[i].x / 2;
			rtv.height = pass->VSMShadowMapsSize[i].y / 2;
			ShadowMaps.push_back(std::make_shared<RenderTarget>(rtv));
		}
	}
}

void M_ShadowVSMRecieverBase::Bind(UINT materialinstance)
{
	RenderingEngine* engine = GetRenderingEngine();
	assert(engine);

	// 定数バッファの設定
	WriteParams(
		(UINT)2, 
		0,
		engine->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	WriteParams(
		(UINT)1, 
		2,
		engine->GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowReciever).hCPU, 
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);

	// テクスチャコピー
	engine->CopyPassTextureSRV(ShadowMaps[ShadowPass::Near].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::NearVSM);
	engine->CopyPassTextureSRV(ShadowMaps[ShadowPass::Middle].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::MiddleVSM);
	engine->CopyPassTextureSRV(ShadowMaps[ShadowPass::Far].get()->GetHandleSRV().hCPU, Material::Shadow, 0, ShadowPass::FarVSM);
}
