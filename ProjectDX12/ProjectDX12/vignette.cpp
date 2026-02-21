
// ImGUI
#include "imgui/imgui.h"

// Scene
#include "SceneManager.h"

// System/Constant
#include "ConstantBuffer.h"
// System/Rendering/Volume
#include "Copy.h"
#include "vignette.h"
// System/Rendering
#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

void Vignette::Init()
{
	Volume::Init(4,2);

	// ルートシグネチャ
	{
		RootSignature::Parameter param[] = 
		{
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		pRootSignatureData = std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		Pipeline::Description desc = {};
		desc.pRootSignature = pRootSignatureData->Get();
		desc.VSFile = L"../game/assets/shader/VS_Sprite.cso";
		desc.PSFile = L"../game/assets/shader/PS_Vignette.cso";
		desc.pInputLayout = Pipeline::IED_POS_TEX;
		desc.InputLayoutNum = Pipeline::IED_POS_TEX_COUNT;
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.RenderTargetNum = 1;
		pPipelineData.push_back(std::make_unique<Pipeline>(desc));
	}
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.pRTVHeap = pRTVHeap.get();
		desc.pSRVHeap = pHeap.get();
		pRTV = std::make_shared<RenderTarget>(desc);
	}
	// パラメーター定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = pHeap.get();
		desc.size = sizeof(VignetteParams);
		Params = std::make_unique<ConstantBuffer>(desc);
	}

	// パラメーター書き込み
	VignetteParam.color = { 0,0,0,0 };
	VignetteParam.start = 0.5f;
	VignetteParam.range = 0.75f;
	Params->Write(&VignetteParam);
}

void Vignette::Draw()
{
	ImGui::Begin("Vignette");
	float color[4];
	memcpy(color, &VignetteParam.color, sizeof(DirectX::XMFLOAT4));
	{
		ImGui::ColorEdit4("color",color);
		ImGui::SliderFloat("start", &VignetteParam.start, 0.01f, 1.0f);
		ImGui::SliderFloat("range", &VignetteParam.range, 0.01f, 1.0f);
	}
	memcpy(&VignetteParam.color,color,sizeof(DirectX::XMFLOAT4));
	ImGui::End();

	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();

	// バッファに書き込み
	Params->Write(&VignetteParam);
	// ポストプロセス用RTVをバインド
	BindPostProcessRTV();
	// 各種オブジェクトをバインド
	BindHeap();
	engine.lock()->CopyGlobalTextureSRV(pRTV.get()->GetHandleSRV().hCPU, GlobalTextureResourceKey::MainTexture);
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] = 
	{
		pRTV.get()->GetHandleSRV().hGPU,
		Params.get()->GetHandle().hGPU,
	};
	BindRootSignature(desc, _countof(desc));
	BindPipeline(0);
	// 描画
	Rendering();

	// MainTextureに張り付け
	pPostProcessRTV->RTV2SRV();
	engine.lock()->GlobalTextureSRV2RTV(GlobalTextureResourceKey::MainTexture);
	Copy::ExecuteCopy(pHeap.get(), pPostProcessRTV.get()->GetHandleSRV().hGPU, engine.lock()->GetGlobalTextureRTV(GlobalTextureResourceKey::MainTexture).hCPU);
	engine.lock()->GlobalTextureRTV2SRV(GlobalTextureResourceKey::MainTexture);
}
