
#include "vignette.h"

#include "ConstantBuffer.h"

#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

#include "Copy.h"

#include "imgui/imgui.h"

void Vignette::Init()
{
	// ルートシグネチャ
	{
		RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::DescriptionTable desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		RootSignatureData = std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.cull = D3D12_CULL_MODE_BACK;
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"assets/shader/VS_Sprite.cso";
		desc.PSFile = L"assets/shader/PS_Vignette.cso";
		desc.pRootSignature = RootSignatureData->Get();
		desc.RenderTargetNum = 1;
		PipelineData = std::make_unique<Pipeline>(desc);
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
		ParamBuf = std::make_unique<ConstantBuffer>(desc);
	}

	// パラメーター書き込み
	Param.color = { 0,0,0,0 };
	Param.start = 0.5f;
	Param.range = 0.5f;
	ParamBuf->Write(&Param);
}

void Vignette::Draw()
{
	ImGui::Begin("Vignette");
	float color[4];
	memcpy(color, &Param.color, sizeof(DirectX::XMFLOAT4));
	{
		ImGui::ColorEdit4("color",color);
		ImGui::SliderFloat("start", &Param.start, 0.01f, 1.0f);
		ImGui::SliderFloat("range", &Param.range, 0.01f, 1.0f);
	}
	memcpy(&Param.color,color,sizeof(DirectX::XMFLOAT4));
	ImGui::End();

	// バッファに書き込み
	ParamBuf->Write(&Param);
	// ポストプロセス用RTVをバインド
	PostProcessRTV->SRV2RTV();
	BindPostProcessRTV();
	// 各種オブジェクトをバインド
	BindPipeline();
	BindHeap();
	// MainTextureを取得
	GetGlobalSRV(RTV, GlobalTextureResourceKey::MainTexture);
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] = {
		RTV.get()->GetHandleSRV().hGPU,
		ParamBuf.get()->GetHandle().hGPU,
	};
	RootSignatureData->Bind(desc, _countof(desc));
	// 描画
	Rendering();

	// MainTextureに張り付け
	PostProcessRTV->RTV2SRV();
	RenderingEngine::GlobalTextureSRV2RTV(GlobalTextureResourceKey::MainTexture);
	Copy::ExecuteCopy(Heap.get(), PostProcessRTV.get(),RenderingEngine::GetGlobalTextureRTV(GlobalTextureResourceKey::MainTexture).hCPU);
	RenderingEngine::GlobalTextureRTV2SRV(GlobalTextureResourceKey::MainTexture);
}
