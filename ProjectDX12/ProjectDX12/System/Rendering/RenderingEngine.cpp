
#include "Component/Rendering/RenderingComponent.h"
#include "GameObject/Camera/CameraBase.h"
#include "GameObject/Light/LightBase.h"
#include "imgui/imguiImage.h"
#include "Scene/SceneBase.h"
#include "Scene/SceneManager.h"
#include "System/GameObject/GameObject.h"
#include "System/Rendering/ConstantBuffer/ConstantWVP.h"
#include "System/Rendering/Pass/CustomDepthNormalPass.h"
#include "System/Rendering/Pass/DepthNormalPass.h"
#include "System/Rendering/Pass/ShadowPass.h"
#include "System/Rendering/Volume/Blur.h"
#include "System/Rendering/Volume/Copy.h"
#include "System/Rendering/Volume/volume.h"
#include "System/Rendering/GlobalResourceKey.h"
#include "System/Rendering/RenderingEngine.h"
#include "System/DirectX.h"
#include "System/Input.h"

void RenderingEngine::Init()
{
	// グローバルディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 256;
		pHeap = std::make_shared<DescriptorHeap>(desc);
	}
	// ディスクリプターヒープ(レンダーターゲット)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = 64;
		pRTVHeap = std::make_shared<DescriptorHeap>(desc);
	}
	// ディスクリプタヒープ（深度バッファ)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num = 32;
		pDSVHeap = std::make_shared<DescriptorHeap>(desc);
	}
	// グローバルリソース
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = pHeap.get();
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::Camera] = std::make_shared<ConstantBuffer>(desc);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::Light] = std::make_shared<ConstantBuffer>(desc);
		desc.size = sizeof(ShadowParam::ShadowMapsParam);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ShadowMaps1] = std::make_shared<ConstantBuffer>(desc);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ShadowMaps2] = std::make_shared<ConstantBuffer>(desc);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ShadowMaps3] = std::make_shared<ConstantBuffer>(desc);
		desc.size = sizeof(ShadowParam::ShadowReceieverParam);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ShadowReciever] = std::make_shared<ConstantBuffer>(desc);
	}
	// グローバルRTV
	{
		// Main
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			desc.pRTVHeap = pRTVHeap.get();
			desc.pSRVHeap = pHeap.get();
			GlobalTexture[GlobalTextureResourceKey::MainTexture] = std::make_shared<RenderTarget>(desc);
		}
		// GBuffer
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			desc.pRTVHeap = pRTVHeap.get();
			desc.pSRVHeap = pHeap.get();
			GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture] = std::make_shared<RenderTarget>(desc);
		}
	}
	// 深度バッファ
	{
		DepthStencil::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.pDSVHeap = pDSVHeap.get();
		DSV = std::make_shared<DepthStencil>(desc);
	}

	// 汎用クラス作成
	Volume::Load();
	Copy::Create();
	Gauss::Create();

	// リソースオブジェクト
	Camera[CameraType::MAIN] = SceneManager::GetCurrentScene()->AddGameObject<CameraDebug>(SceneBase::Layer::Camera);
	Camera[CameraType::MAIN]->SetIsMain(true);
	Camera[CameraType::FRUSTUM] = SceneManager::GetCurrentScene()->AddGameObject<CameraDebug>(SceneBase::Layer::Camera);
	Light = SceneManager::GetCurrentScene()->AddGameObject<LightBase>(SceneBase::Layer::Environment);

	// レンダリングパス
	ShadowMapsPass = std::make_shared<ShadowPass>();
	ShadowMapsPass->Init(pRTVHeap, pHeap, pDSVHeap);
	ODepthNormalPass = std::make_unique<OpaqueDepthNormalPass>();
	ODepthNormalPass->Init(pRTVHeap, pHeap, pDSVHeap);

	// ポストプロセス
	ObjectPostProcess = std::make_unique<PostProcess>();
	CanvasPostProcess = std::make_unique<PostProcess>();

	// ディファードシェーダー
	SetupDefferedShader();
}

void RenderingEngine::Uninit()
{
	Gauss::Destroy();
	Copy::Destroy();
	Volume::Unload();
}

void RenderingEngine::Update()
{
	if (Input::GetKeyTrigger(VK_SHIFT))
	{
		CurrentCameraType = CameraType::FRUSTUM;
		Camera[CameraType::MAIN]->SetIsMain(false);
	}
	else if(Input::GetKeyRelease(VK_SHIFT))
	{
		CurrentCameraType = CameraType::MAIN;
		Camera[CameraType::MAIN]->SetIsMain(true);
	}

	ObjectPostProcess->Update();
	CanvasPostProcess->Update();
}

void RenderingEngine::Draw()
{
	// 表示領域の設定
	SetViewPort(WINDOW_WIDTH,WINDOW_HEIGHT);

	// デプスクリア
	DSV->Clear();

	// 環境情報の設定
	{
		// カメラ
		Camera[CurrentCameraType]->DrawUpdateSpeed();
		DirectX::XMFLOAT4X4 camera;
		camera._11 = Camera[CurrentCameraType]->GetPosition().x;
		camera._12 = Camera[CurrentCameraType]->GetPosition().y;
		camera._13 = Camera[CurrentCameraType]->GetPosition().z;
		camera._14 = 0.0f;
		DirectX::XMFLOAT3 cameraForward = Camera[CurrentCameraType]->GetForward();
		camera._21 = cameraForward.x;
		camera._22 = cameraForward.y;
		camera._23 = cameraForward.z;
		camera._24 = 0.0f;
		WriteGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera, &camera);

		// ライト
		DirectX::XMFLOAT4X4 light;
		light._11 = Light->GetPosition().x;
		light._12 = Light->GetPosition().y;
		light._13 = Light->GetPosition().z;
		light._14 = 0;
		light._21 = Light->GetDirection().x;
		light._22 = Light->GetDirection().y;
		light._23 = Light->GetDirection().z;
		light._24 = 0;
		light._31 = Light->GetColor().x;
		light._32 = Light->GetColor().y;
		light._33 = Light->GetColor().z;
		light._34 = Light->GetColor().w;
		light._41 = Light->GetPower();
		light._42 = Light->GetAmbient();
		WriteGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Light, &light);
	}

	// シャドウマップ描画
	CurrentRenderingTiming = Material::RenderingTiming::Shadow;
	ShadowMapsRendering();
	// 不透明デプス描画
	CurrentRenderingTiming = Material::RenderingTiming::OpaqueDepthNormal;
	OpaqueDepthNormalRendering();
	// 不透明デプス描画後パス
	CurrentRenderingTiming = Material::RenderingTiming::AfterOpaqueDepthNormal;
	AfterOpaqueDepthNormalRendering();
	// 環境描画
	CurrentRenderingTiming = Material::Environment;
	EnvironmentRendering();
	// ディファード描画
	CurrentRenderingTiming = Material::Deffered;
	DefferedRendering();
	DefferedLighting();
	// フォワード描画
	CurrentRenderingTiming = Material::Forward;
	ForwardRendering();
	// 不透明デプス描画
	CurrentRenderingTiming = Material::RenderingTiming::TranslucentDepthNormal;
	TranslucentDepthNormalRendering();
	// その他描画
	CurrentRenderingTiming = Material::RenderingTiming::Other;
	ObjectPostProcessRendering();
	CanvasPostProcessRendering();
	// メインテクスチャへのコピー
	Copy::ExecuteCopy(pHeap.get(), GlobalTexture[GlobalTextureResourceKey::MainTexture].get()->GetHandleSRV().hGPU, GetBBuffer());
	// バッファ描画
	ViewShadowMaps();
	ViewDepthNormal();
	ViewGBuffers();
	ViewPasses();

	// 最終的にバックバッファに描画
	auto hRTV = GetBBuffer();
	SetRenderTarget(1, &hRTV, DSV->GetHandleDSV().hCPU);

	// 登録された情報をクリア
	EnvironmentObjects.clear();
	DefferedObjects.clear();
	ForwardObjects.clear();
}

void RenderingEngine::CopyTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE src, D3D12_CPU_DESCRIPTOR_HANDLE dest)
{
	GetDevice()->CopyDescriptorsSimple(
		1,
		dest,
		src,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
}

DescriptorHeap::Handle RenderingEngine::GetGlobalConstantBufferResource(UINT key)
{
	if (!GlobalConstantBuffer.contains(key))
		return DescriptorHeap::Handle();
	return GlobalConstantBuffer[key]->GetHandle();
}

void RenderingEngine::WriteGlobalConstantBufferResource(UINT key, void* data)
{
	GlobalConstantBuffer[key]->Write(data);
}

std::shared_ptr<RenderTarget> RenderingEngine::GetGlobalRenderTarget(UINT key)
{
	if (!GlobalTexture.contains(key))
		return nullptr;
	return GlobalTexture[key];
}

DescriptorHeap::Handle RenderingEngine::GetGlobalTextureRTV(UINT key)
{
	if (!GlobalTexture.contains(key))
		return DescriptorHeap::Handle();
	return GlobalTexture[key]->GetHandleRTV();
}

DescriptorHeap::Handle RenderingEngine::GetGlobalTextureSRV(UINT key)
{
	if (!GlobalTexture.contains(key))
		return DescriptorHeap::Handle();
	return GlobalTexture[key]->GetHandleSRV();
}

void RenderingEngine::GlobalTextureRTV2SRV(UINT key)
{
	if (!GlobalTexture.contains(key))
		return;
	GlobalTexture[key]->RTV2SRV();
}

void RenderingEngine::GlobalTextureSRV2RTV(UINT key)
{
	if (!GlobalTexture.contains(key))
		return;
	GlobalTexture[key]->SRV2RTV();
}

void RenderingEngine::CopyGlobalTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT key)
{
	GetDevice()->CopyDescriptorsSimple(
		1,
		dest,
		GetGlobalTextureSRV(key).hCPU,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
}

void RenderingEngine::SetupDefferedShader()
{
	// ルートシグネチャ
	{
		RootSignature::Parameter param[] =
		{
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		DefferedLightingShader.pRootSignatureData = std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		PipelineState::InputLayout layout[] =
		{
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
		};
		PipelineState::Description desc = {};
		desc.VSFile = L"../game/assets/shader/VS_Sprite.cso";
		desc.PSFile = L"../game/assets/shader/PS_DefferedLighting.cso";
		desc.pRootSignature = DefferedLightingShader.pRootSignatureData->Get();
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.RenderTargetNum = 1;
		DefferedLightingShader.pPipelineData = std::make_unique<PipelineState>(desc);
	}
	// パラメーター定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = pHeap.get();
		// Params
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		DefferedLightingShader.Params.push_back(std::make_unique<ConstantBuffer>(desc)); // カメラ
		DefferedLightingShader.Params.push_back(std::make_unique<ConstantBuffer>(desc)); // ライト
		DefferedLightingShader.Params.push_back(std::make_unique<ConstantBuffer>(desc)); // VPINV
	}
}


void RenderingEngine::AddRenderObject(
	GameObject& obj,
	UINT timing,
	UINT passType
)
{
	RenderingInfo info = { obj };
	switch (timing)
	{
	case Material::Shadow:
		ShadowMapsPass->AddObj(obj);
		break;
	case Material::OpaqueDepthNormal:
		ODepthNormalPass->AddObj(obj);
		break;
	case Material::AfterOpaqueDepthNormal:
		if (RenderingPasses.contains(timing) && RenderingPasses[timing].contains(passType))
			RenderingPasses[timing][passType]->AddObj(obj);
		break;
	case Material::Environment:
		EnvironmentObjects.push_back(info);
		break;
	case Material::Deffered:
		DefferedObjects.push_back(info);
		break;
	case Material::Forward:
		ForwardObjects.push_back(info);
		break;
	case Material::TranslucentDepthNormal:
		break;
	case Material::Canvas:
		break;
	case Material::Other:
		break;
	default:
		break;
	}
}

std::shared_ptr<ShadowPass> RenderingEngine::GetShadowMapsPass()
{
	return std::dynamic_pointer_cast<ShadowPass>(ShadowMapsPass);
}

void RenderingEngine::CopyPassTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT timing, UINT type, UINT idx)
{
	std::shared_ptr<RenderTarget> src = GetPassTexture(timing, type, idx);
	GetDevice()->CopyDescriptorsSimple(
		1,
		dest,
		src->GetHandleSRV().hCPU,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
}

std::shared_ptr<RenderTarget> RenderingEngine::GetPassTexture(UINT timing, UINT type, UINT idx)
{
	switch (timing)
	{
	case Material::Shadow:
		return ShadowMapsPass->GetTexture(idx);
		break;
	case Material::OpaqueDepthNormal:
		return ODepthNormalPass->GetTexture(idx);
		break;
	case Material::AfterOpaqueDepthNormal:
		if (RenderingPasses.contains(timing) && RenderingPasses[timing].contains(type))
			return RenderingPasses[timing][type]->GetTexture(idx);
		break;
	case Material::Deffered:
		return nullptr;
		break;
	case Material::Forward:
		return nullptr;
		break;
	case Material::TranslucentDepthNormal:
		return nullptr;
		break;
	case Material::Canvas:
		return nullptr;
		break;
	case Material::Other:
		return nullptr;
		break;
	default:
		return nullptr;
		break;
	}
}

void RenderingEngine::RegisterRenderingComponentRef(std::shared_ptr<RenderingComponent> component)
{
	RenderingComponents.push_back(component);
}

void RenderingEngine::RegisterMaterialRef(std::shared_ptr<Material> material)
{
	RenderingMaterials.push_back(material);
}

void RenderingEngine::ShadowMapsRendering()
{
	ShadowMapsPass->Execute();
}

void RenderingEngine::OpaqueDepthNormalRendering()
{
	ODepthNormalPass->Execute();
}

void RenderingEngine::AfterOpaqueDepthNormalRendering()
{
	if (!RenderingPasses.contains(Material::RenderingTiming::AfterOpaqueDepthNormal))
		return;
	auto it = RenderingPasses[Material::RenderingTiming::AfterOpaqueDepthNormal].begin();
	for (; it != RenderingPasses[Material::RenderingTiming::AfterOpaqueDepthNormal].end(); it++)
	{
		it->second->Execute();
	}
}

void RenderingEngine::EnvironmentRendering()
{
	static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0 };

	// ターゲット化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->SRV2RTV();

	// RTVの設定
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->Clear(clearColor);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		GlobalTexture[GlobalTextureResourceKey::MainTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs);

	// 環境描画
	for (int i = 0; i < EnvironmentObjects.size(); ++i)
	{
		EnvironmentObjects[i].obj.RenderingBase();
	}

	// リソース化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->RTV2SRV();
}

void RenderingEngine::DefferedRendering()
{
	static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0 };

	// ターゲット化
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture + i]->SRV2RTV();
	}

	// RTVの設定
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture + i]->Clear(clearColor);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

	// ディファードレンダリング
	for (int i = 0; i < DefferedObjects.size(); ++i)
	{
		DefferedObjects[i].obj.RenderingBase();
	}

	// リソース化
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture + i]->RTV2SRV();
	}
}

void RenderingEngine::DefferedLighting()
{
	// ターゲット化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->SRV2RTV();

	// RTVの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		GlobalTexture[GlobalTextureResourceKey::MainTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs);

	// 定数バッファ構築
	GetDevice()->CopyDescriptorsSimple(
		(UINT)2,
		DefferedLightingShader.Params[0]->GetHandle().hCPU,
		GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	DirectX::XMFLOAT4X4 vpInv = CameraBase::GetMainViewProjectionInvMatrix();
	DefferedLightingShader.Params[2]->Write(&vpInv);

	// 各種オブジェクトをバインド
	ID3D12DescriptorHeap* heaps[] =
	{
		pHeap->Get(),
	};
	DescriptorHeap::Bind(heaps, 1);
	DefferedLightingShader.pPipelineData->Bind();

	// MainTextureを取得
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] = 
	{
		GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture].get()->GetHandleSRV().hGPU,
		ODepthNormalPass->GetTextureSRV(OpaqueDepthNormalPass::NormalTexture).hGPU,
		ODepthNormalPass->GetTextureSRV(OpaqueDepthNormalPass::DepthTexture).hGPU,
		DefferedLightingShader.Params[0].get()->GetHandle().hGPU,
		DefferedLightingShader.Params[1].get()->GetHandle().hGPU,
		DefferedLightingShader.Params[2].get()->GetHandle().hGPU,
	};
	DefferedLightingShader.pRootSignatureData->Bind(desc, _countof(desc));

	// 描画
	Copy::ExecuteScreenDraw();

	// リソース化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->RTV2SRV();
}

void RenderingEngine::ForwardRendering()
{
	// ターゲット化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->SRV2RTV();

	// RTVの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		GlobalTexture[GlobalTextureResourceKey::MainTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

	// フォワードレンダリング
	for (int i = 0; i < ForwardObjects.size(); ++i)
	{
		ForwardObjects[i].obj.RenderingBase();
	}

	// リソース化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->RTV2SRV();
}

void RenderingEngine::TranslucentDepthNormalRendering()
{

}

void RenderingEngine::ObjectPostProcessRendering()
{
	ObjectPostProcess->Draw();
}

void RenderingEngine::CanvasPostProcessRendering()
{
	CanvasPostProcess->Draw();
}

void RenderingEngine::ViewShadowMaps()
{
	static const ImVec2 size = { 480,480 };

	ImGui::Begin("ShadowMaps");
	{
		ImGui::Text("ShadowMap1");
		ImGui::Image(ImGUIImage::GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Near).get()), size);
		ImGui::Text("ShadowMap2");
		ImGui::Image(ImGUIImage::GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Middle).get()), size);
		ImGui::Text("ShadowMap3");
		ImGui::Image(ImGUIImage::GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Far).get()), size);
		ImGui::Text("VSMShadowMap1");
		ImGui::Image(ImGUIImage::GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::NearVSM).get()), size);
		ImGui::Text("VSMShadowMap2");
		ImGui::Image(ImGUIImage::GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::MiddleVSM).get()), size);
		ImGui::Text("VSMShadowMap3");
		ImGui::Image(ImGUIImage::GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::FarVSM).get()), size);
	}
	ImGui::End();
}

void RenderingEngine::ViewDepthNormal()
{
	ImGui::Begin("DepthNormal");
	{
		ImGui::Text("Depth");
		ImGui::Image(ImGUIImage::GetImage(
			pHeap.get(),
			ODepthNormalPass->GetTexture(OpaqueDepthNormalPass::DepthTexture).get()), { 240,135 });
		ImGui::Text("Normal");
		ImGui::Image(ImGUIImage::GetImage(
			pHeap.get(),
			ODepthNormalPass->GetTexture(OpaqueDepthNormalPass::NormalTexture).get()), { 240,135 });
	}
	ImGui::End();
}

void RenderingEngine::ViewGBuffers()
{
	ImGui::Begin("GBuffers");
	{
		ImGui::Text("DefferedAlbedo");
		ImGui::Image(ImGUIImage::GetImage(
			pHeap.get(), 
			GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture].get()), { 240,135 });
	}
	ImGui::End();
}

void RenderingEngine::ViewPasses()
{
	ImGui::Begin("Passes");
	{
		ImGui::Text("CustomDepth");
		ImGui::Image(ImGUIImage::GetImage(
			pHeap.get(),
			RenderingPasses[Material::RenderingTiming::AfterOpaqueDepthNormal][RenderingPass::CustomDepthNormal]->
			GetTexture(CustomDepthNormalPass::DepthTexture).get()), {240,135});
	}
	ImGui::End();
}
