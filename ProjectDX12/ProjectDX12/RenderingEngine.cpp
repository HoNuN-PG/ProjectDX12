
#include "GlobalResourceKey.h"
#include "DebugImGUI.h"

#include "RenderingEngine.h"
#include "DirectX.h"

#include "volume.h"
#include "Copy.h"

#include "GameObject.h"
#include "CameraBase.h"
#include "LightBase.h"

#include "SceneManager.h"
#include "SceneBase.h"

#include "ShadowPass.h"
#include "DepthNormalPass.h"
#include "CustomDepthNormalPass.h"

#include "ConstantWVP.h"

std::shared_ptr<DescriptorHeap>	RenderingEngine::GlobalHeap;
std::unordered_map<UINT, std::shared_ptr<ConstantBuffer>> RenderingEngine::GlobalConstantBuffer;
std::unordered_map<UINT, std::shared_ptr<RenderTarget>>	RenderingEngine::GlobalTexture;
Material::RenderingTiming RenderingEngine::CurrentRenderingTiming;

void RenderingEngine::Init()
{
	// ボリュームの作成
	Volume::Load();
	Copy::Load();

	// グローバルディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 256;
		GlobalHeap = std::make_shared<DescriptorHeap>(desc);
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
		desc.pHeap = GlobalHeap.get();
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::Camera] = std::make_shared<ConstantBuffer>(desc);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::Light] = std::make_shared<ConstantBuffer>(desc);
		desc.size = sizeof(ShadowParam::ShadowMapsParam);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ShadowMaps] = std::make_shared<ConstantBuffer>(desc);
		desc.size = sizeof(ShadowParam::ShadowReceieveParam);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ShadowReceive] = std::make_shared<ConstantBuffer>(desc);
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
			desc.pSRVHeap = GlobalHeap.get();
			GlobalTexture[GlobalTextureResourceKey::MainTexture] = std::make_shared<RenderTarget>(desc);
		}
		// DepthNormal
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.format = DXGI_FORMAT_R32G32_FLOAT;
			desc.pRTVHeap = RTVHeap.get();
			desc.pSRVHeap = GlobalHeap.get();
			GlobalTexture[GlobalTextureResourceKey::DepthTexture] = std::make_shared<RenderTarget>(desc);
			desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
			GlobalTexture[GlobalTextureResourceKey::NormalTexture] = std::make_shared<RenderTarget>(desc);
		}
		// GBuffer
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			desc.pRTVHeap = RTVHeap.get();
			desc.pSRVHeap = GlobalHeap.get();
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
		DSV = std::make_unique<DepthStencil>(desc);
	}

	// リソースオブジェクト
	Camera = SceneManager::GetCurrentScene()->AddGameObject<CameraDebug>(SceneBase::Layer::Camera);
	Light = SceneManager::GetCurrentScene()->AddGameObject<LightBase>(SceneBase::Layer::Environment);

	// レンダリングパス
	ShadowMapsPass = std::make_unique<ShadowPass>();
	ShadowMapsPass->Init(RTVHeap, GlobalHeap, DSVHeap);
	ODepthNormalPass = std::make_unique<OpaqueDepthNormalPass>();

	// ポストプロセス
	ObjectPostProcess = std::make_unique<PostProcess>();
	CanvasPostProcess = std::make_unique<PostProcess>();
}

void RenderingEngine::Uninit()
{
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

	WriteGlobalConstantBufferResource();
	CurrentRenderingTiming = Material::RenderingTiming::Shadow;
	ShadowMapsRendering();
	CurrentRenderingTiming = Material::RenderingTiming::OpaqueDepthNormal;
	OpaqueDepthNormalRendering();
	CurrentRenderingTiming = Material::RenderingTiming::AfterOpaqueDepthNormal;
	AfterOpaqueDepthNormalRendering();
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
	Copy::Copy::ExecuteCopy(GlobalHeap.get(), GlobalTexture[GlobalTextureResourceKey::MainTexture].get(), GetRTV());
	ViewShadowMaps();
	ViewDepthNormal();
	ViewGBuffers();
	ViewPasses();
	EndRendering();

	// 最終的にバックバッファに描画
	auto hRTV = GetRTV();
	SetRenderTarget(1, &hRTV, DSV->GetHandleDSV().hCPU);

	// 登録された情報をクリア
	ForwardObjects.clear();
	DefferedObjects.clear();
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

void RenderingEngine::WriteGlobalConstantBufferResource()
{
	// カメラ
	DirectX::XMFLOAT4X4 camera;
	camera._11 = CameraDebug::m_MainPos.x;
	camera._12 = CameraDebug::m_MainPos.y;
	camera._13 = CameraDebug::m_MainPos.z;
	camera._14 = 0.0f;
	GlobalConstantBuffer[GlobalConstantBufferResourceKey::Camera]->Write(&camera);
	// ライト
	DirectX::XMFLOAT4X4 light;
	light._11 = Light->GetDir().x;
	light._12 = Light->GetDir().y;
	light._13 = Light->GetDir().z;
	light._14 = Light->GetPower();
	light._21 = Light->GetPos().x;
	light._22 = Light->GetPos().y;
	light._23 = Light->GetPos().z;
	light._24 = Light->GetAmbient();
	light._31 = Light->GetColor().x;
	light._32 = Light->GetColor().y;
	light._33 = Light->GetColor().z;
	light._34 = Light->GetColor().w;

	GlobalConstantBuffer[GlobalConstantBufferResourceKey::Light]->Write(&light);
}

std::shared_ptr<RenderTarget> RenderingEngine::GetPassTexture(UINT timing, UINT type, UINT idx)
{
	switch (timing)
	{
	case Material::Shadow:
		ShadowMapsPass->GetTexture(idx);
		break;
	case Material::OpaqueDepthNormal:
		break;
	case Material::AfterOpaqueDepthNormal:
		if (RenderingPasses.contains(timing) && RenderingPasses[timing].contains(type))
			return RenderingPasses[timing][type]->GetTexture(idx);
		break;
	case Material::Deffered:
		break;
	case Material::Forward:
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
	static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0 };

	// ターゲット化
	GlobalTexture[GlobalTextureResourceKey::DepthTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	GlobalTexture[GlobalTextureResourceKey::NormalTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// RTVの設定
	GlobalTexture[GlobalTextureResourceKey::DepthTexture]->Clear(clearColor);
	GlobalTexture[GlobalTextureResourceKey::NormalTexture]->Clear(clearColor);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		GlobalTexture[GlobalTextureResourceKey::DepthTexture]->GetHandleRTV().hCPU,
		GlobalTexture[GlobalTextureResourceKey::NormalTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

	// DepthNormal
	ODepthNormalPass->Execute();

	// リソース化
	GlobalTexture[GlobalTextureResourceKey::DepthTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GlobalTexture[GlobalTextureResourceKey::NormalTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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
	static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0 };

	// ターゲット化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// RTVの設定
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->Clear(clearColor);
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
	// ターゲット化
	GlobalTexture[GlobalTextureResourceKey::DepthTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	GlobalTexture[GlobalTextureResourceKey::NormalTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// RTVの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		GlobalTexture[GlobalTextureResourceKey::DepthTexture]->GetHandleRTV().hCPU,
		GlobalTexture[GlobalTextureResourceKey::NormalTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

	// DepthNormal
	//for (int i = 0; i < RenderObjects[Material::RenderingTiming::O_DEPTH_NORMAL_TIMING].size(); ++i)
	//{
	//	RenderObjects[Material::RenderingTiming::O_DEPTH_NORMAL_TIMING][i].obj.RenderingBase();
	//}

	// リソース化
	GlobalTexture[GlobalTextureResourceKey::DepthTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GlobalTexture[GlobalTextureResourceKey::NormalTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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
	ImGui::Begin("ShadowMaps");
	{
		ImGui::Text("ShadowMap1");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			GlobalHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Near).get()), { 240,135 });
		ImGui::Text("ShadowMap2");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			GlobalHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Middle).get()), { 240,135 });
		ImGui::Text("ShadowMap3");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			GlobalHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Far).get()), { 240,135 });
	}
	ImGui::End();
}

void RenderingEngine::ViewDepthNormal()
{
	ImGui::Begin("DepthNormal");
	{
		ImGui::Text("Depth");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			GlobalHeap.get(),
			GlobalTexture[GlobalTextureResourceKey::DepthTexture].get()), { 240,135 });
		ImGui::Text("Normal");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			GlobalHeap.get(),
			GlobalTexture[GlobalTextureResourceKey::NormalTexture].get()), { 240,135 });
	}
	ImGui::End();
}

void RenderingEngine::ViewGBuffers()
{
	ImGui::Begin("GBuffers");
	{
		ImGui::Text("DefferedAlbedo");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			GlobalHeap.get(), 
			GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture].get()), { 240,135 });
		ImGui::Text("DefferedNormal");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			GlobalHeap.get(),
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
			GlobalHeap.get(),
			RenderingPasses[Material::RenderingTiming::AfterOpaqueDepthNormal][RenderingPass::CustomDepthNormal]->
			GetTexture(CustomDepthNormalPass::CustomDepth).get()), {240,135});
	}
	ImGui::End();
}

void RenderingEngine::EndRendering()
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