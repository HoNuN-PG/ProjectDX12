
#include "RenderingEngine.h"

#include "DebugImGUI.h"

#include "GlobalResourceKey.h"
#include "ConstantWVP.h"

#include "DirectX.h"
#include "SceneManager.h"
#include "SceneBase.h"
#include "GameObject.h"
#include "CameraBase.h"
#include "LightBase.h"

#include "ShadowPass.h"
#include "DepthNormalPass.h"
#include "CustomDepthNormalPass.h"

#include "volume.h"
#include "Copy.h"
#include "Blur.h"

void RenderingEngine::Init()
{
	// グローバルディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 256;
		RenderingHeap = std::make_shared<DescriptorHeap>(desc);
	}
	// ディスクリプターヒープ(レンダーターゲット)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = 64;
		RTVHeap = std::make_shared<DescriptorHeap>(desc);
	}
	// ディスクリプタヒープ（深度バッファ)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num = 32;
		DSVHeap = std::make_shared<DescriptorHeap>(desc);
	}
	// グローバルリソース
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = RenderingHeap.get();
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
			desc.pRTVHeap = RTVHeap.get();
			desc.pSRVHeap = RenderingHeap.get();
			GlobalTexture[GlobalTextureResourceKey::MainTexture] = std::make_shared<RenderTarget>(desc);
		}
		// GBuffer
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			desc.pRTVHeap = RTVHeap.get();
			desc.pSRVHeap = RenderingHeap.get();
			GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture] = std::make_shared<RenderTarget>(desc);
			desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
			GlobalTexture[GlobalTextureResourceKey::DefferedNormalTexture] = std::make_shared<RenderTarget>(desc);
		}
	}
	// 深度バッファ
	{
		DepthStencil::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.pDSVHeap = DSVHeap.get();
		DSV = std::make_shared<DepthStencil>(desc);
	}

	// 汎用クラス作成
	Volume::Load();
	Copy::Create();
	Gauss::Create();

	// リソースオブジェクト
	Camera = SceneManager::GetCurrentScene()->AddGameObject<CameraDebug>(SceneBase::Layer::Camera);
	Light = SceneManager::GetCurrentScene()->AddGameObject<LightBase>(SceneBase::Layer::Environment);

	// レンダリングパス
	ShadowMapsPass = std::make_unique<ShadowPass>();
	ShadowMapsPass->Init(RTVHeap, RenderingHeap, DSVHeap);
	ODepthNormalPass = std::make_unique<OpaqueDepthNormalPass>();
	ODepthNormalPass->Init(RTVHeap, RenderingHeap, DSVHeap);

	// ポストプロセス
	ObjectPostProcess = std::make_unique<PostProcess>();
	CanvasPostProcess = std::make_unique<PostProcess>();
}

void RenderingEngine::Uninit()
{
	Gauss::Destroy();
	Copy::Destroy();
	Volume::Unload();
}

void RenderingEngine::Update()
{
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
		DirectX::XMFLOAT4X4 camera;
		camera._11 = CameraBase::m_MainPos.x;
		camera._12 = CameraBase::m_MainPos.y;
		camera._13 = CameraBase::m_MainPos.z;
		camera._14 = 0.0f;
		DirectX::XMFLOAT3 cameraForward = Camera->GetForward();
		camera._21 = cameraForward.x;
		camera._22 = cameraForward.y;
		camera._23 = cameraForward.z;
		camera._24 = 0.0f;
		WriteGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera,&camera);

		// ライト
		DirectX::XMFLOAT4X4 light;
		light._11 = Light->GetPos().x;
		light._12 = Light->GetPos().y;
		light._13 = Light->GetPos().z;
		light._14 = 0;
		light._21 = Light->GetDir().x;
		light._22 = Light->GetDir().y;
		light._23 = Light->GetDir().z;
		light._24 = 0;
		light._31 = Light->GetColor().x;
		light._32 = Light->GetColor().y;
		light._33 = Light->GetColor().z;
		light._34 = Light->GetColor().w;
		light._41 = Light->GetPower();
		light._42 = Light->GetAmbient();
		WriteGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Light, &light);
	}

	CurrentRenderingTiming = Material::RenderingTiming::Shadow;
	ShadowMapsRendering();
	CurrentRenderingTiming = Material::RenderingTiming::OpaqueDepthNormal;
	OpaqueDepthNormalRendering();
	CurrentRenderingTiming = Material::RenderingTiming::AfterOpaqueDepthNormal;
	AfterOpaqueDepthNormalRendering();
	CurrentRenderingTiming = Material::Environment;
	EnvironmentRendering();
	CurrentRenderingTiming = Material::Deffered;
	DefferedRendering();
	DefferedLighting();
	CurrentRenderingTiming = Material::Forward;
	ForwardRendering();
	CurrentRenderingTiming = Material::RenderingTiming::TranslucentDepthNormal;
	TranslucentDepthNormalRendering();
	CurrentRenderingTiming = Material::RenderingTiming::Other;
	ObjectPostProcessRendering();
	CanvasPostProcessRendering();
	static int i = -1;
	// Gauss::ExecuteScreenGauss2(i,{WINDOW_WIDTH,WINDOW_HEIGHT}, GlobalTexture[GlobalTextureResourceKey::MainTexture],GlobalTexture[GlobalTextureResourceKey::MainTexture]);
	Copy::ExecuteCopy(RenderingHeap.get(), GlobalTexture[GlobalTextureResourceKey::MainTexture].get()->GetHandleSRV().hGPU, GetRTV());
	ViewShadowMaps();
	ViewDepthNormal();
	ViewGBuffers();
	ViewPasses();
	RefreshRendering();

	// 最終的にバックバッファに描画
	auto hRTV = GetRTV();
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
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

DescriptorHeap::Handle RenderingEngine::GetGlobalConstantBufferResource(UINT key)
{
	if (!GlobalConstantBuffer.contains(key))
		return DescriptorHeap::Handle();
	return GlobalConstantBuffer[key]->GetHandle();
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

void RenderingEngine::CopyGlobalTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT key)
{
	GetDevice()->CopyDescriptorsSimple(
		1,
		dest,
		GetGlobalTextureSRV(key).hCPU,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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

void RenderingEngine::WriteGlobalConstantBufferResource(UINT key, void* data)
{
	GlobalConstantBuffer[key]->Write(data);
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

void RenderingEngine::CopyPassTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT timing, UINT type, UINT idx)
{
	std::shared_ptr<RenderTarget> src = GetPassTexture(timing,type,idx);
	GetDevice()->CopyDescriptorsSimple(
		1,
		dest,
		src->GetHandleSRV().hCPU,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void RenderingEngine::AddRenderObject(
	GameObject& obj, 
	UINT timing, 
	UINT passType)
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
		if(RenderingPasses.contains(timing) && RenderingPasses[timing].contains(passType))
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

void RenderingEngine::AddRenderingMaterial(std::shared_ptr<Material> material)
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
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// RTVの設定
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->Clear(clearColor);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		GlobalTexture[GlobalTextureResourceKey::MainTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs);

	// 環境描画
	for (int i = 0; i < EnvironmentObjects.size(); ++i)
	{
		EnvironmentObjects[i].obj.RenderingBase();
	}

	// リソース化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void RenderingEngine::DefferedRendering()
{
	static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0 };

	// ターゲット化
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture + i]->ResourceBarrier(
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	// RTVの設定
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture + i]->Clear(clearColor);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture]->GetHandleRTV().hCPU,
		GlobalTexture[GlobalTextureResourceKey::DefferedNormalTexture]->GetHandleRTV().hCPU,
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
		GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture + i]->ResourceBarrier(
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void RenderingEngine::DefferedLighting()
{

}

void RenderingEngine::ForwardRendering()
{
	// ターゲット化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// RTVの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		GlobalTexture[GlobalTextureResourceKey::MainTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

	// フォワードレンダリング
	for (int i = 0; i < ForwardObjects.size(); ++i)
	{
		ForwardObjects[i].obj.RenderingBase();
	}

	// リソース化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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
	static const ImVec2 size = { 480,270 };

	ImGui::Begin("ShadowMaps");
	{
		ImGui::Text("ShadowMap1");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Near).get()), size);
		ImGui::Text("ShadowMap2");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Middle).get()), size);
		ImGui::Text("ShadowMap3");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Far).get()), size);
		ImGui::Text("VSMShadowMap1");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::NearVSM).get()), size);
		ImGui::Text("VSMShadowMap2");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::MiddleVSM).get()), size);
		ImGui::Text("VSMShadowMap3");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::FarVSM).get()), size);
	}
	ImGui::End();
}

void RenderingEngine::ViewDepthNormal()
{
	ImGui::Begin("DepthNormal");
	{
		ImGui::Text("Depth");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(),
			ODepthNormalPass->GetTexture(OpaqueDepthNormalPass::DepthTexture).get()), { 240,135 });
		ImGui::Text("Normal");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(),
			ODepthNormalPass->GetTexture(OpaqueDepthNormalPass::NormalTexture).get()), { 240,135 });
	}
	ImGui::End();
}

void RenderingEngine::ViewGBuffers()
{
	ImGui::Begin("GBuffers");
	{
		ImGui::Text("DefferedAlbedo");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(), 
			GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture].get()), { 240,135 });
		ImGui::Text("DefferedNormal");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(),
			GlobalTexture[GlobalTextureResourceKey::DefferedNormalTexture].get()), { 240,135 });
	}
	ImGui::End();
}

void RenderingEngine::ViewPasses()
{
	ImGui::Begin("Passes");
	{
		ImGui::Text("CustomDepth");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			RenderingHeap.get(),
			RenderingPasses[Material::RenderingTiming::AfterOpaqueDepthNormal][RenderingPass::CustomDepthNormal]->
			GetTexture(CustomDepthNormalPass::DepthTexture).get()), {240,135});
	}
	ImGui::End();
}

void RenderingEngine::RefreshRendering()
{
	RenderingMaterials.remove_if(
		[](std::weak_ptr<Material> object)
		{
			return object.expired();
		});
	for (auto material : RenderingMaterials)
	{
		material.lock()->EndRendering();
	}
}