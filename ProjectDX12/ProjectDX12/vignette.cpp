
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
		RootSignatureData = std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		Pipeline::InputLayout layout[] = 
		{
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.VSFile = L"../exe/assets/shader/VS_Sprite.cso";
		desc.PSFile = L"../exe/assets/shader/PS_Vignette.cso";
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.pRootSignature = RootSignatureData->Get();
		desc.RenderTargetNum = 1;
		PipelineData.push_back(std::make_unique<Pipeline>(desc));
	}
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.pRTVHeap = RTVHeap.get();
		desc.pSRVHeap = Heap.get();
		RTV = std::make_shared<RenderTarget>(desc);
	}
	// パラメーター定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = Heap.get();
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
	BindPipeline(0);
	BindHeap();
	// MainTextureを取得
	engine.lock()->CopyGlobalTextureSRV(RTV.get()->GetHandleSRV().hCPU, GlobalTextureResourceKey::MainTexture);
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] = {
		RTV.get()->GetHandleSRV().hGPU,
		Params.get()->GetHandle().hGPU,
	};
	RootSignatureData->Bind(desc, _countof(desc));
	// 描画
	Rendering();

	// MainTextureに張り付け
	PostProcessRTV->RTV2SRV();
	engine.lock()->GlobalTextureSRV2RTV(GlobalTextureResourceKey::MainTexture);
	Copy::ExecuteCopy(Heap.get(), PostProcessRTV.get()->GetHandleSRV().hGPU, engine.lock()->GetGlobalTextureRTV(GlobalTextureResourceKey::MainTexture).hCPU);
	engine.lock()->GlobalTextureRTV2SRV(GlobalTextureResourceKey::MainTexture);
}
