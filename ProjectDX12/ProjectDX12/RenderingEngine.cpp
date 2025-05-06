
#include "GlobalResourceKey.h"
#include "StartUp.h"

#include "RenderingEngine.h"
#include "DirectX.h"
#include "GameObject.h"
#include "CameraBase.h"
#include "LightBase.h"

#include "SceneManager.h"
#include "SceneBase.h"

#include "ConstantWVP.h"

#include "RenderingComponent.h"

std::unique_ptr<DescriptorHeap>	RenderingEngine::GlobalHeap;
std::unordered_map<UINT, std::unique_ptr<ConstantBuffer>> RenderingEngine::GlobalConstantBuffer;
std::unordered_map<UINT, std::unique_ptr<RenderTarget>>	RenderingEngine::GlobalTexture;

void RenderingEngine::Init()
{
	// グローバルディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 64;
		GlobalHeap = std::make_unique<DescriptorHeap>(desc);
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
		desc.num = 1;
		DSVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// グローバルリソース
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = GlobalHeap.get();
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ScreenWVP] = std::make_unique<ConstantBuffer>(desc);
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::Camera] = std::make_unique<ConstantBuffer>(desc);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::Light] = std::make_unique<ConstantBuffer>(desc);
	}
	// GBuffer
	{
		for (int i = 0; i < MAX_GBUFFER; i++)
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.pRTVHeap = RTVHeap.get();
			desc.pSRVHeap = GlobalHeap.get();
			GlobalTexture[GlobalTextureResourceKey::AlbedoTexture + i] = std::make_unique<RenderTarget>(desc);
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

	m_RenderObjects.resize(GameObject::RenderingTiming::MAX_TIMING);
}

void RenderingEngine::Uninit()
{
}

void RenderingEngine::Update()
{
}

void RenderingEngine::Draw()
{
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	// 表示領域の設定
	D3D12_VIEWPORT vp = { 0, 0, 1280.0f, 720.0f, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, 1280.0f, 720.0f };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	// デプスクリア
	DSV->Clear();

	WriteGlobalConstantBufferResource();
	DefferedRendering();
	DefferedLighting();
	ForwardRendering();
	ViewGBuffers();

	// 登録された情報をクリア
	for (int i = 0; i < m_RenderObjects.size(); ++i)
	{
		m_RenderObjects[i].clear();
	}
}

DescriptorHeap::Handle RenderingEngine::GetGlobalConstantBufferResource(UINT key)
{
	if (!GlobalConstantBuffer.contains(key))
		return DescriptorHeap::Handle();
	return GlobalConstantBuffer[key].get()->GetHandle();
}

DescriptorHeap::Handle RenderingEngine::GetGlobalTextureResource(UINT key)
{
	if (!GlobalTexture.contains(key))
		return DescriptorHeap::Handle();
	return GlobalTexture[key].get()->GetHandleSRV();
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
	m_RenderObjects[timing].push_back(info);
}

void RenderingEngine::DefferedRendering()
{
	static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0 };

	// ターゲット化
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		GlobalTexture[GlobalTextureResourceKey::AlbedoTexture + i]->ResourceBarrier(
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	// RTVの設定
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		GlobalTexture[GlobalTextureResourceKey::AlbedoTexture + i]->Clear(clearColor);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		GlobalTexture[GlobalTextureResourceKey::AlbedoTexture]->GetHandleRTV().hCPU,
		GlobalTexture[GlobalTextureResourceKey::NormalTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

	// ディファードレンダリング
	for (int i = 0; i < m_RenderObjects[GameObject::RenderingTiming::DEFERRED].size(); ++i)
	{
		m_RenderObjects[GameObject::RenderingTiming::DEFERRED][i].obj.Rendering();
	}

	// リソース化
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		GlobalTexture[GlobalTextureResourceKey::AlbedoTexture + i]->ResourceBarrier(
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void RenderingEngine::DefferedLighting()
{
}

void RenderingEngine::ForwardRendering()
{
	// バックバッファに描画
	auto hRTV = GetRTV();
	SetRenderTarget(1, &hRTV, DSV->GetHandleDSV().hCPU);

	// フォワードレンダリング
	for (int i = 0; i < m_RenderObjects[GameObject::RenderingTiming::FORWARD].size(); ++i)
	{
		m_RenderObjects[GameObject::RenderingTiming::FORWARD][i].obj.Rendering();
	}
}

void RenderingEngine::ViewGBuffers()
{
	ImGui::Begin("GBuffers");
	{
		ImGui::Image(GetDebugImGUI()->GetImGUIImage(
			GlobalHeap.get(), 
			GlobalConstantBuffer[GlobalConstantBufferResourceKey::ScreenWVP].get(), 
			GlobalTexture[GlobalTextureResourceKey::AlbedoTexture].get()), {480,270});
		ImGui::Image(GetDebugImGUI()->GetImGUIImage(
			GlobalHeap.get(),
			GlobalConstantBuffer[GlobalConstantBufferResourceKey::ScreenWVP].get(),
			GlobalTexture[GlobalTextureResourceKey::NormalTexture].get()), { 480,270 });
	}
	ImGui::End();

	// バックバッファに描画
	auto hRTV = GetRTV();
	SetRenderTarget(1, &hRTV, DSV->GetHandleDSV().hCPU);
}
