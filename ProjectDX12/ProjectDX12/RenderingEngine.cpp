
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

#include "ConstantWVP.h"

std::unique_ptr<DescriptorHeap>	RenderingEngine::GlobalHeap;
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
		desc.num = 64;
		GlobalHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 256;
		Heap = std::make_unique<DescriptorHeap>(desc);
	}
	// ディスクリプターヒープ(レンダーターゲット)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = 64;
		RTVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ディスクリプタヒープ（深度バッファ)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num = 1;
		DSVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// グローバルリソース
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = GlobalHeap.get();
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ScreenWVP] = std::make_shared<ConstantBuffer>(desc);
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::Camera] = std::make_shared<ConstantBuffer>(desc);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::Light] = std::make_shared<ConstantBuffer>(desc);
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

	// ScreenWVPの設定
	GlobalConstantBuffer[GlobalConstantBufferResourceKey::ScreenWVP]->Write(ConstantWVP::Calc2DMatrix(
		{ WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2, 0 },
		{ 0,0,0 },
		{ WINDOW_WIDTH	  ,  WINDOW_HEIGHT	  , 0 }));
	// リソースオブジェクト
	Camera = SceneManager::GetCurrentScene()->AddGameObject<CameraDebug>();
	Light = SceneManager::GetCurrentScene()->AddGameObject<LightBase>();

	RenderObjects.resize(Material::RenderingTiming::MAX_TIMING);
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
	CurrentRenderingTiming = Material::DEPTH_NORMAL;
	DepthNormalRendering();
	CurrentRenderingTiming = Material::DEFERRED;
	DefferedRendering();
	DefferedLighting();
	CurrentRenderingTiming = Material::FORWARD;
	ForwardRendering();
	ObjectPostProcessRendering();
	CanvasPostProcessRendering();
	Copy::Copy::ExecuteCopy(GlobalHeap.get(), GlobalTexture[GlobalTextureResourceKey::MainTexture].get(), GetRTV());
	ViewDepthNormal();
	ViewGBuffers();

	// 最終的にバックバッファに描画
	auto hRTV = GetRTV();
	SetRenderTarget(1, &hRTV, DSV->GetHandleDSV().hCPU);

	// 登録された情報をクリア
	for (int i = 0; i < RenderObjects.size(); ++i)
	{
		RenderObjects[i].clear();
	}
}

DescriptorHeap::Handle RenderingEngine::GetGlobalConstantBufferResource(UINT key)
{
	if (!GlobalConstantBuffer.contains(key))
		return DescriptorHeap::Handle();
	return GlobalConstantBuffer[key]->GetHandle();
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
	light._21 = Light->GetColor().x;
	light._22 = Light->GetColor().y;
	light._23 = Light->GetColor().z;
	light._24 = Light->GetAmbient();
	GlobalConstantBuffer[GlobalConstantBufferResourceKey::Light]->Write(&light);
}

void RenderingEngine::AddRenderObject(GameObject& obj, int timing)
{
	RenderingInfo info = { obj };
	RenderObjects[timing].push_back(info);
}

void RenderingEngine::DepthNormalRendering()
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
	for (int i = 0; i < RenderObjects[Material::RenderingTiming::DEPTH_NORMAL].size(); ++i)
	{
		RenderObjects[Material::RenderingTiming::DEPTH_NORMAL][i].obj.RenderingBase();
	}

	// リソース化
	GlobalTexture[GlobalTextureResourceKey::DepthTexture]->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GlobalTexture[GlobalTextureResourceKey::NormalTexture]->ResourceBarrier(
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
	for (int i = 0; i < RenderObjects[Material::RenderingTiming::DEFERRED].size(); ++i)
	{
		RenderObjects[Material::RenderingTiming::DEFERRED][i].obj.RenderingBase();
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
	for (int i = 0; i < RenderObjects[Material::RenderingTiming::FORWARD].size(); ++i)
	{
		RenderObjects[Material::RenderingTiming::FORWARD][i].obj.RenderingBase();
	}

	// リソース化
	GlobalTexture[GlobalTextureResourceKey::MainTexture]->ResourceBarrier(
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

void RenderingEngine::ViewDepthNormal()
{
	ImGui::Begin("DepthNormal");
	{
		ImGui::Text("Depth");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			GlobalHeap.get(),
			GlobalConstantBuffer[GlobalConstantBufferResourceKey::ScreenWVP].get(),
			GlobalTexture[GlobalTextureResourceKey::DepthTexture].get()), { 240,135 });
		ImGui::Text("Normal");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			GlobalHeap.get(),
			GlobalConstantBuffer[GlobalConstantBufferResourceKey::ScreenWVP].get(),
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
			GlobalConstantBuffer[GlobalConstantBufferResourceKey::ScreenWVP].get(), 
			GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture].get()), { 240,135 });
		ImGui::Text("DefferedNormal");
		ImGui::Image(DebugImGUI::GetImGUIImage(
			GlobalHeap.get(),
			GlobalConstantBuffer[GlobalConstantBufferResourceKey::ScreenWVP].get(),
			GlobalTexture[GlobalTextureResourceKey::DefferedNormalTexture].get()), { 240,135 });
	}
	ImGui::End();
}
