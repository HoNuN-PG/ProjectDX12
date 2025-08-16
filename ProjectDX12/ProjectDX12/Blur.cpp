
#include "Blur.h"

#include "DescriptorHeap.h"
#include "RenderTarget.h"
#include "ConstantBuffer.h"

#include "volume.h"
#include "Copy.h"

std::shared_ptr<DescriptorHeap>	Gauss::Heap;
std::shared_ptr<DescriptorHeap>	Gauss::RTVHeap;
std::unique_ptr<MeshBuffer>	Gauss::Screen;
std::unique_ptr<RootSignature> Gauss::RootSignatureData;
std::vector<std::unique_ptr<Pipeline>> Gauss::PipelineData;
std::vector<std::unique_ptr<RenderTarget>> Gauss::GaussRTVs;
BlurParam::GaussParam Gauss::Weights;
std::vector<std::unique_ptr<ConstantBuffer>> Gauss::Params;

void Gauss::Load()
{
	// スクリーン頂点
	Vertex screenVtx[] =
	{
		{{-0.5f, 0.5f,0} ,{0,0}} ,
		{{ 0.5f, 0.5f,0} ,{1,0}} ,
		{{-0.5f,-0.5f,0} ,{0,1}} ,
		{{ 0.5f,-0.5f,0} ,{1,1}} ,
	};

	// スクリーン
	MeshBuffer::Description desc = {};
	desc.pVtx = screenVtx;
	desc.vtxSize = sizeof(Vertex);
	desc.vtxCount = _countof(screenVtx);
	desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	Screen = std::make_unique<MeshBuffer>(desc);

	// ボリュームディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 8;
		Heap = std::make_shared<DescriptorHeap>(desc);
	}
	// ボリュームディスクリプターヒープ(レンダーターゲット)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = 8;
		RTVHeap = std::make_shared<DescriptorHeap>(desc);
	}
	// ルートシグネチャ
	{
		RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
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
		PipelineData.resize(GaussPipelineType::Pipeline_MAX);

		Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.cull = D3D12_CULL_MODE_BACK;
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.pRootSignature = RootSignatureData->Get();
		desc.RenderTargetNum = 1;

		desc.VSFile = L"assets/shader/VS_BlurX.cso";
		desc.PSFile = L"assets/shader/PS_Blur.cso";
		PipelineData[GaussPipelineType::XBlurPipeline] = std::make_unique<Pipeline>(desc);

		desc.VSFile = L"assets/shader/VS_BlurY.cso";
		desc.PSFile = L"assets/shader/PS_Blur.cso";
		PipelineData[GaussPipelineType::YBlurPipeline] = std::make_unique<Pipeline>(desc);
	}
	// GaussRTVs
	{
		GaussRTVs.resize(GaussRTVsType::RTVs_MAX);

		RenderTarget::Description desc = {};
		desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.pRTVHeap = RTVHeap.get();
		desc.pSRVHeap = Heap.get();

		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		GaussRTVs[GaussRTVsType::Buffer] = std::make_unique<RenderTarget>(desc);

		desc.width = WINDOW_WIDTH / 2;
		desc.height = WINDOW_HEIGHT;
		GaussRTVs[GaussRTVsType::XBlur] = std::make_unique<RenderTarget>(desc);

		desc.width = WINDOW_WIDTH / 2;
		desc.height = WINDOW_HEIGHT / 2;
		GaussRTVs[GaussRTVsType::YBlur] = std::make_unique<RenderTarget>(desc);
	}
	// パラメーター定数バッファ
	{
		Params.resize(GaussParamsType::Params_MAX);

		ConstantBuffer::Description desc = {};
		desc.pHeap = Heap.get();

		desc.size = sizeof(BlurParam::ScreenParam);
		Params[GaussParamsType::ScreenX] = std::make_unique<ConstantBuffer>(desc);
		Params[GaussParamsType::ScreenY] = std::make_unique<ConstantBuffer>(desc);

		desc.size = sizeof(float) * BlurParam::GAUSS_WEIGHTS;
		Params[GaussParamsType::GaussWeights] = std::make_unique<ConstantBuffer>(desc);
	}

	// GaussWeights
	Weights.weights = std::make_shared<float[]>(BlurParam::GAUSS_WEIGHTS);
	CalcWeights(Weights.weights,25);
}

void Gauss::ExecuteGauss(std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest,
	std::weak_ptr<float[]> weights)
{
	// スクリーンサイズ
	BlurParam::ScreenParam p1(GaussRTVs[GaussRTVsType::XBlur]->Width, GaussRTVs[GaussRTVsType::XBlur]->Height);
	BlurParam::ScreenParam p2(GaussRTVs[GaussRTVsType::YBlur]->Width, GaussRTVs[GaussRTVsType::YBlur]->Height);
	Params[GaussParamsType::ScreenX]->Write(&p1);
	Params[GaussParamsType::ScreenY]->Write(&p2);
	// 重みの設定
	if (!weights.expired())
	{
		float w[BlurParam::GAUSS_WEIGHTS];
		for (int i = 0; i < BlurParam::GAUSS_WEIGHTS; ++i) { w[i] = weights.lock()[i]; }
		Params[GaussParamsType::GaussWeights]->Write(&w);
	}
	else
	{
		float w[BlurParam::GAUSS_WEIGHTS];
		for (int i = 0; i < BlurParam::GAUSS_WEIGHTS; ++i) { w[i] = Weights.weights[i]; }
		Params[GaussParamsType::GaussWeights]->Write(&w);
	}

	// レンダーターゲット切り替え
	SetViewPort(GaussRTVs[GaussRTVsType::XBlur]->Width, GaussRTVs[GaussRTVsType::XBlur]->Height);
	GaussRTVs[GaussRTVsType::XBlur]->SRV2RTV();
	GaussRTVs[GaussRTVsType::XBlur]->Clear();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs1[] = {
		GaussRTVs[GaussRTVsType::XBlur]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs1);
	// XBlur
	PipelineData[GaussPipelineType::XBlurPipeline]->Bind();
	ID3D12DescriptorHeap* heaps1[] =
	{
		Heap->Get(),
	};
	DescriptorHeap::Bind(heaps1, 1);
	Volume::CopyTextureSRV(src->GetHandleSRV().hCPU,GaussRTVs[GaussRTVsType::Buffer]->GetHandleSRV().hCPU);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen1[] = {
		Params[GaussParamsType::ScreenX]->GetHandle().hGPU,
		GaussRTVs[GaussRTVsType::Buffer]->GetHandleSRV().hGPU,
		Params[GaussParamsType::GaussWeights]->GetHandle().hGPU,
	};
	RootSignatureData->Bind(hScreen1, _countof(hScreen1));
	Screen->Draw();
	GaussRTVs[GaussRTVsType::XBlur]->RTV2SRV();

	// レンダーターゲット切り替え
	SetViewPort(GaussRTVs[GaussRTVsType::YBlur]->Width, GaussRTVs[GaussRTVsType::YBlur]->Height);
	GaussRTVs[GaussRTVsType::YBlur]->SRV2RTV();
	GaussRTVs[GaussRTVsType::YBlur]->Clear();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs2[] = {
		GaussRTVs[GaussRTVsType::YBlur]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs2);
	// YBlur
	PipelineData[GaussPipelineType::YBlurPipeline]->Bind();
	ID3D12DescriptorHeap* heaps2[] =
	{
		Heap->Get(),
	};
	DescriptorHeap::Bind(heaps2, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen2[] = {
		Params[GaussParamsType::ScreenY]->GetHandle().hGPU,
		GaussRTVs[GaussRTVsType::XBlur]->GetHandleSRV().hGPU,
		Params[GaussParamsType::GaussWeights]->GetHandle().hGPU,
	};
	RootSignatureData->Bind(hScreen2, _countof(hScreen2));
	Screen->Draw();
	GaussRTVs[GaussRTVsType::YBlur]->RTV2SRV();

	// コピー
	Copy::ExecuteCopy(Heap.get(), GaussRTVs[GaussRTVsType::YBlur]->GetHandleSRV().hGPU, dest);
}

void Gauss::CalcWeights(std::weak_ptr<float[]> weights, float blur)
{
	// 重みの合計
	float total = 0;

	// ここからガウス関数を用いて重みを計算している
	// ループ変数のxが基準テクセルからの距離
	for (int x = 0; x < BlurParam::GAUSS_WEIGHTS; x++)
	{
		weights.lock()[x] = expf((float)(-x * x) / (2 * blur * blur));
		total += 2.0f * weights.lock()[x];
	}

	// 重みの合計で除算することで、重みの合計を1にしている
	for (int i = 0; i < BlurParam::GAUSS_WEIGHTS; i++)
	{
		weights.lock()[i] /= total;
	}
}
