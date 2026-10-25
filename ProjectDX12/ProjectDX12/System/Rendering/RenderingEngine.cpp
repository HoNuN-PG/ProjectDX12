
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

// ShadowPassの完全な型が必要なためcpp側で定義する
RenderingEngine::RenderingEngine() = default;
RenderingEngine::~RenderingEngine() = default;

void RenderingEngine::Init()
{
	// グローバル定数バッファディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.staging = TRUE;
		desc.num = STAGING_HEAP_SIZE;
		pStagingHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// グローバルディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = HEAP_SIZE;
		pHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ディスクリプターヒープ(レンダーターゲット)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = RTV_HEAP_SIZE;
		pRTVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ディスクリプタヒープ（深度バッファ)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num = DSV_HEAP_SIZE;
		pDSVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// グローバル定数バッファリソース
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = pStagingHeap.get();
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::Camera] = std::make_unique<ConstantBuffer>(desc);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::Light] = std::make_unique<ConstantBuffer>(desc);
		desc.size = sizeof(ShadowParam::ShadowMapsParam);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ShadowMaps1] = std::make_unique<ConstantBuffer>(desc);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ShadowMaps2] = std::make_unique<ConstantBuffer>(desc);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ShadowMaps3] = std::make_unique<ConstantBuffer>(desc);
		desc.size = sizeof(ShadowParam::ShadowReceieverParam);
		GlobalConstantBuffer[GlobalConstantBufferResourceKey::ShadowReciever] = std::make_unique<ConstantBuffer>(desc);
	}
	// グローバルRTV
	{
		GlobalTextureFormat[GlobalTextureResourceKey::MainTexture] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		GlobalTextureFormat[GlobalTextureResourceKey::DefferedAlbedoTexture] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	}
	{
		// Main
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.format = GlobalTextureFormat[GlobalTextureResourceKey::MainTexture];
			desc.pRTVHeap = pRTVHeap.get();
			desc.pSRVHeap = pStagingHeap.get();
			GlobalTextureStaging[GlobalTextureResourceKey::MainTexture] = std::make_unique<RenderTarget>(desc);
			desc.pSRVHeap = pHeap.get();
			GlobalTexture[GlobalTextureResourceKey::MainTexture] = std::make_unique<RenderTarget>(desc);
		}
		// GBuffer
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.format = GlobalTextureFormat[GlobalTextureResourceKey::DefferedAlbedoTexture];
			desc.pRTVHeap = pRTVHeap.get();
			desc.pSRVHeap = pStagingHeap.get();
			GlobalTextureStaging[GlobalTextureResourceKey::DefferedAlbedoTexture] = std::make_unique<RenderTarget>(desc);
			desc.pSRVHeap = pHeap.get();
			GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture] = std::make_unique<RenderTarget>(desc);
		}
	}
	// 深度バッファ
	{
		DepthStencil::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.pDSVHeap = pDSVHeap.get();
		DSV = std::make_unique<DepthStencil>(desc);
	}

	// ディファードシェーダー
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
			RootSignature::Description rootsignature =
			{
				param,
				_countof(param)
			};
			DefferedLightingShader.pRootSignatureData = std::make_unique<RootSignature>(rootsignature);
		}
		// パイプライン
		{
			std::vector<DXGI_FORMAT> formats = { GlobalTextureFormat[GlobalTextureResourceKey::DefferedAlbedoTexture] };
			PipelineState::Description desc = {
				L"",
				L"",
				L"../game/assets/shader/VS_Sprite.cso",
				L"../game/assets/shader/PS_DefferedLighting.cso",
				DefferedLightingShader.pRootSignatureData->Get(),
				PipelineState::IED_POS_TEX,
				PipelineState::IED_POS_TEX_COUNT,
				D3D12_CULL_MODE_BACK,
				1,
				formats,
				FALSE,
			};

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

	// 汎用クラス作成
	Volume::Load();
	pCopy = std::make_unique<Copy>();
	pGauss = std::make_unique<Gauss>(*pCopy);

	// リソースオブジェクト
	Camera[CameraType::MAIN] = SceneManager::GetCurrentScene()->AddGameObject<CameraDebug>(SceneBase::Layer::Camera);
	Camera[CameraType::MAIN]->SetIsMain(true);
	Camera[CameraType::FRUSTUM] = SceneManager::GetCurrentScene()->AddGameObject<CameraDebug>(SceneBase::Layer::Camera);
	Light = SceneManager::GetCurrentScene()->AddGameObject<LightBase>(SceneBase::Layer::Environment);

	// レンダリングパス
	ShadowMapsPass = std::make_unique<ShadowPass>(*this);
	ShadowMapsPass->Init(pRTVHeap.get(), pStagingHeap.get(), pHeap.get(), pDSVHeap.get());
	ODepthNormalPass = std::make_unique<OpaqueDepthNormalPass>(*this);
	ODepthNormalPass->Init(pRTVHeap.get(), pStagingHeap.get(), pHeap.get(), pDSVHeap.get());

	// ポストプロセス
	ObjectPostProcess = std::make_unique<PostProcess>();
	CanvasPostProcess = std::make_unique<PostProcess>();
}

void RenderingEngine::Uninit()
{
	RenderingPasses.clear();
	CanvasPostProcess.reset();
	ObjectPostProcess.reset();
	ODepthNormalPass.reset();
	ShadowMapsPass.reset();
	pGauss.reset();
	pCopy.reset();
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
		Camera[CurrentCameraType]->DrawImGUI();
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
	// ステージングから読み取り専用テクスチャへコピー
	CopyTexture(GlobalTextureStaging[GlobalTextureResourceKey::MainTexture].get()->GetHandleSRV().hCPU, GlobalTexture[GlobalTextureResourceKey::MainTexture].get()->GetHandleSRV().hCPU);
	// メインテクスチャへのコピー
	pCopy->ExecuteCopy2BBuffer(pHeap.get(), GlobalTexture[GlobalTextureResourceKey::MainTexture].get()->GetHandleSRV().hGPU);
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

void RenderingEngine::CopyTexture(D3D12_CPU_DESCRIPTOR_HANDLE src, D3D12_CPU_DESCRIPTOR_HANDLE dest)
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
	{
		return DescriptorHeap::Handle();	
	}
	return GlobalConstantBuffer[key]->GetHandle();
}

void RenderingEngine::WriteGlobalConstantBufferResource(UINT key, void* data)
{
	auto it = GlobalConstantBuffer.find(key);
	assert(it != GlobalConstantBuffer.end());
	it->second->Write(data);
}

RenderTarget* RenderingEngine::GetGlobalStagingRenderTarget(UINT key)
{
	auto it = GlobalTextureStaging.find(key);
	return it != GlobalTextureStaging.end() ? it->second.get() : nullptr;
}

RenderTarget* RenderingEngine::GetGlobalRenderTarget(UINT key)
{
	auto it = GlobalTexture.find(key);
	return it != GlobalTexture.end() ? it->second.get() : nullptr;
}

void RenderingEngine::CopyGlobalTexture(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT key)
{
	RenderTarget* src = GetGlobalStagingRenderTarget(key);
	assert(src);
	CopyTexture(src->GetHandleSRV().hCPU, dest);
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
	case Material::Environment:
		EnvironmentObjects.push_back(info);
		break;
	case Material::Deffered:
		DefferedObjects.push_back(info);
		break;
	case Material::Forward:
		ForwardObjects.push_back(info);
		break;
	default:
		// パスで描画するタイミング
		if (RenderingPass* pass = FindPass(timing, passType))
		{
			pass->AddObj(obj);	
		}
		break;
	}
}

ShadowPass* RenderingEngine::GetShadowMapsPass()
{
	return ShadowMapsPass.get();
}

RenderingPass* RenderingEngine::FindPass(UINT timing, UINT type)
{
	// シャドウと不透明深度法線はタイミングごとに固定のパス（種類は見ない）
	switch (timing)
	{
	case Material::Shadow:
		return ShadowMapsPass.get();
	case Material::OpaqueDepthNormal:
		return ODepthNormalPass.get();
	default:
		break;
	}

	auto passes = RenderingPasses.find(timing);
	if (passes == RenderingPasses.end())
	{
		return nullptr;	
	}
	auto pass = passes->second.find(type);
	return pass != passes->second.end() ? pass->second.get() : nullptr;
}

std::shared_ptr<RenderTarget> RenderingEngine::GetPassTextureStaging(UINT timing, UINT type, UINT idx)
{
	RenderingPass* pass = FindPass(timing, type);
	return pass ? pass->GetTextureStaging(idx) : nullptr;
}

std::shared_ptr<RenderTarget> RenderingEngine::GetPassTexture(UINT timing, UINT type, UINT idx)
{
	RenderingPass* pass = FindPass(timing, type);
	return pass ? pass->GetTexture(idx) : nullptr;
}

void RenderingEngine::CopyPassTexture(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT timing, UINT type, UINT idx)
{
	std::shared_ptr<RenderTarget> src = GetPassTextureStaging(timing, type, idx);
	assert(src);
	CopyTexture(src->GetHandleSRV().hCPU, dest);
}

std::vector<DXGI_FORMAT> RenderingEngine::GetPassFormat(UINT timing, UINT type)
{
	if (RenderingPass* pass = FindPass(timing, type))
	{
		return pass->GetPassFormat();	
	}

	// パスを持たないタイミングはメインテクスチャに描画する
	switch (timing)
	{
	case Material::Environment:
	case Material::Deffered:
	case Material::Forward:
	case Material::TranslucentDepthNormal:
	case Material::Canvas:
	case Material::Other:
		return { GlobalTextureFormat[GlobalTextureResourceKey::MainTexture] };
	default:
		return {};
	}
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
	auto passes = RenderingPasses.find(Material::RenderingTiming::AfterOpaqueDepthNormal);
	if (passes == RenderingPasses.end())
	{
		return;	
	}
	for (auto& pass : passes->second)
	{
		pass.second->Execute();
	}
}

void RenderingEngine::EnvironmentRendering()
{
	static const float clearColor[4] = { 0, 0, 0, 0 };

	// ターゲット化
	GlobalTextureStaging[GlobalTextureResourceKey::MainTexture]->SRV2RTV();

	// RTVの設定
	GlobalTextureStaging[GlobalTextureResourceKey::MainTexture]->Clear(clearColor);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		GlobalTextureStaging[GlobalTextureResourceKey::MainTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs);

	// 環境描画
	for (int i = 0; i < EnvironmentObjects.size(); ++i)
	{
		EnvironmentObjects[i].obj.RenderingBase();
	}

	// リソース化
	GlobalTextureStaging[GlobalTextureResourceKey::MainTexture]->RTV2SRV();
}

void RenderingEngine::DefferedRendering()
{
	static const float clearColor[4] = { 0, 0, 0, 0 };

	// ターゲット化
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		GlobalTextureStaging[GlobalTextureResourceKey::DefferedAlbedoTexture + i]->SRV2RTV();
	}

	// RTVの設定
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		GlobalTextureStaging[GlobalTextureResourceKey::DefferedAlbedoTexture + i]->Clear(clearColor);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		GlobalTextureStaging[GlobalTextureResourceKey::DefferedAlbedoTexture]->GetHandleRTV().hCPU,
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
		GlobalTextureStaging[GlobalTextureResourceKey::DefferedAlbedoTexture + i]->RTV2SRV();
	}

	// ステージングから読み取り専用テクスチャへコピー
	for (int i = 0; i < MAX_GBUFFER; i++)
	{
		CopyTexture(GlobalTextureStaging[GlobalTextureResourceKey::DefferedAlbedoTexture + i].get()->GetHandleSRV().hCPU, GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture + i].get()->GetHandleSRV().hCPU);
	}
}

void RenderingEngine::DefferedLighting()
{
	// ターゲット化
	GlobalTextureStaging[GlobalTextureResourceKey::MainTexture]->SRV2RTV();

	// RTVの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		GlobalTextureStaging[GlobalTextureResourceKey::MainTexture]->GetHandleRTV().hCPU,
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
	pCopy->ExecuteScreenDraw();

	// リソース化
	GlobalTextureStaging[GlobalTextureResourceKey::MainTexture]->RTV2SRV();
}

void RenderingEngine::ForwardRendering()
{
	// ターゲット化
	GlobalTextureStaging[GlobalTextureResourceKey::MainTexture]->SRV2RTV();

	// RTVの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		GlobalTextureStaging[GlobalTextureResourceKey::MainTexture]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

	// フォワードレンダリング
	for (int i = 0; i < ForwardObjects.size(); ++i)
	{
		ForwardObjects[i].obj.RenderingBase();
	}

	// リソース化
	GlobalTextureStaging[GlobalTextureResourceKey::MainTexture]->RTV2SRV();
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
		ImGui::Image(ImGUIImage::GetInstance()->GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Near).get()), size);
		ImGui::Text("ShadowMap2");
		ImGui::Image(ImGUIImage::GetInstance()->GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Middle).get()), size);
		ImGui::Text("ShadowMap3");
		ImGui::Image(ImGUIImage::GetInstance()->GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::Far).get()), size);
		ImGui::Text("VSMShadowMap1");
		ImGui::Image(ImGUIImage::GetInstance()->GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::NearVSM).get()), size);
		ImGui::Text("VSMShadowMap2");
		ImGui::Image(ImGUIImage::GetInstance()->GetImage(
			pHeap.get(),
			ShadowMapsPass->GetTexture(ShadowPass::MiddleVSM).get()), size);
		ImGui::Text("VSMShadowMap3");
		ImGui::Image(ImGUIImage::GetInstance()->GetImage(
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
		ImGui::Image(ImGUIImage::GetInstance()->GetImage(
			pHeap.get(),
			ODepthNormalPass->GetTexture(OpaqueDepthNormalPass::DepthTexture).get()), { 240,135 });
		ImGui::Text("Normal");
		ImGui::Image(ImGUIImage::GetInstance()->GetImage(
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
		ImGui::Image(ImGUIImage::GetInstance()->GetImage(
			pHeap.get(), 
			GlobalTexture[GlobalTextureResourceKey::DefferedAlbedoTexture].get()), { 240,135 });
	}
	ImGui::End();
}

void RenderingEngine::ViewPasses()
{
	ImGui::Begin("Passes");
	RenderingPass* customDepthNormal = 
		GetRenderingPass<RenderingPass>(Material::RenderingTiming::AfterOpaqueDepthNormal, RenderingPass::CustomDepthNormal);
	if (customDepthNormal)
	{
		ImGui::Text("CustomDepthNormal");
		ImGui::Image(
			ImGUIImage::GetInstance()->GetImage(
				pHeap.get(),
				customDepthNormal->GetTexture(CustomDepthNormalPass::DepthTexture).get()
				), 
{240,135}
			);
	}
	ImGui::End();
}
