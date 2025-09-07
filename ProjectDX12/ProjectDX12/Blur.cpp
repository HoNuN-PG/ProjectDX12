
#include "Blur.h"

#include "RenderingEngine.h"
#include "DescriptorHeap.h"

#include "volume.h"
#include "Copy.h"

std::unique_ptr<Gauss> Gauss::Instance;

void Gauss::Create()
{
	if (!Instance)
	{
		Instance = std::make_unique<Gauss>();

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
		Instance->Screen = std::make_unique<MeshBuffer>(desc);

		// ボリュームディスクリプタヒープ
		{
			DescriptorHeap::Description desc = {};
			desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.num = BlurParam::GAUSS_MAX * GaussRTVType::RTV_MAX + BlurParam::GAUSS_MAX * (GaussParamsType::Params_MAX - 1);
			Instance->Heap = std::make_shared<DescriptorHeap>(desc);
		}
		// ボリュームディスクリプターヒープ(レンダーターゲット)
		{
			DescriptorHeap::Description desc = {};
			desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.num = BlurParam::GAUSS_MAX * GaussRTVType::RTV_MAX;
			Instance->RTVHeap = std::make_shared<DescriptorHeap>(desc);
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
			Instance->RootSignatureData = std::make_unique<RootSignature>(desc);
		}
		// パイプライン
		{
			Instance->PipelineData.resize(GaussPipelineType::Pipeline_MAX);

			Pipeline::InputLayout layout[] = {
				{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
				{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
			};
			Pipeline::Description desc = {};
			desc.cull = D3D12_CULL_MODE_BACK;
			desc.pInputLayout = layout;
			desc.InputLayoutNum = _countof(layout);
			desc.pRootSignature = Instance->RootSignatureData->Get();
			desc.RenderTargetNum = 1;

			desc.VSFile = L"assets/shader/VS_BlurX.cso";
			desc.PSFile = L"assets/shader/PS_Blur.cso";
			Instance->PipelineData[GaussPipelineType::XBlurPipeline] = std::make_unique<Pipeline>(desc);

			desc.VSFile = L"assets/shader/VS_BlurY.cso";
			desc.PSFile = L"assets/shader/PS_Blur.cso";
			Instance->PipelineData[GaussPipelineType::YBlurPipeline] = std::make_unique<Pipeline>(desc);
		}
		// パラメーター定数バッファ
		{
			ConstantBuffer::Description desc = {};
			desc.pHeap = Instance->Heap.get();
			desc.size = sizeof(float) * BlurParam::GAUSS_WEIGHTS;
			Instance->GaussParam = std::make_unique<ConstantBuffer>(desc);
		}

		// GaussWeights
		Instance->Weights.weights = std::make_shared<float[]>(BlurParam::GAUSS_WEIGHTS);
		CalcWeights(Instance->Weights.weights, 1);
		// 重みの設定
		{
			float w[BlurParam::GAUSS_WEIGHTS];
			for (int i = 0; i < BlurParam::GAUSS_WEIGHTS; ++i) { w[i] = Instance->Weights.weights[i]; }
			Instance->GaussParam->Write(&w);
		}
	}
}

void Gauss::Destroy()
{
	if (Instance)
	{
		Instance.reset(nullptr);
	}
}

void Gauss::ExecuteScreenGauss2(int& gaussIdx, DirectX::XMFLOAT2 screen, 
	std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest)
{
	static const int split = 2;

	// GaussRTVs
	if (gaussIdx < 0)
	{
		// Idx計算
		gaussIdx = Instance->GaussRTVs.size() / 3;
		// RTV
		{
			RenderTarget::Description desc = {};
			desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			desc.pRTVHeap = Instance->RTVHeap.get();
			desc.pSRVHeap = Instance->Heap.get();

			desc.width = screen.x;
			desc.height = screen.y;
			Instance->GaussRTVs.push_back(std::make_unique<RenderTarget>(desc));

			desc.width = screen.x / split;
			desc.height = screen.y;
			Instance->GaussRTVs.push_back(std::make_unique<RenderTarget>(desc));

			desc.width = screen.x / split;
			desc.height = screen.y / split;
			Instance->GaussRTVs.push_back(std::make_unique<RenderTarget>(desc));
		}
		// ConstantBuffer
		{
			ConstantBuffer::Description desc = {};
			desc.pHeap = Instance->Heap.get();
			desc.size = sizeof(BlurParam::ScreenParam);
			Instance->Params.push_back(std::make_unique<ConstantBuffer>(desc));
			Instance->Params.push_back(std::make_unique<ConstantBuffer>(desc));

			// スクリーンサイズ
			BlurParam::ScreenParam p1(screen.x, screen.y / split,1);
			BlurParam::ScreenParam p2(screen.x / split, screen.y / split,1);
			Instance->Params[gaussIdx * 2]->Write(&p1);
			Instance->Params[gaussIdx * 2 + 1]->Write(&p2);
		}
	}

	// ビューポート設定
	UINT xIdx = GaussRTVType::XBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	SetViewPort(Instance->GaussRTVs[xIdx]->Width, Instance->GaussRTVs[xIdx]->Height);

	// リソース設定
	Instance->GaussRTVs[xIdx]->SRV2RTV();
	Instance->GaussRTVs[xIdx]->Clear();

	// RTV設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs1[] = {
		Instance->GaussRTVs[xIdx]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs1);

	// XBlur
	UINT bIdx = GaussRTVType::Buffer + (gaussIdx * GaussRTVType::RTV_MAX);
	Instance->PipelineData[GaussPipelineType::XBlurPipeline]->Bind();
	ID3D12DescriptorHeap* heaps1[] =
	{
		Instance->Heap->Get(),
	};
	DescriptorHeap::Bind(heaps1, 1);
	RenderingEngine::CopyTextureSRV(src->GetHandleSRV().hCPU, Instance->GaussRTVs[bIdx]->GetHandleSRV().hCPU);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen1[] = {
		Instance->Params[gaussIdx * 2]->GetHandle().hGPU,
		Instance->GaussRTVs[bIdx]->GetHandleSRV().hGPU,
		Instance->GaussParam->GetHandle().hGPU,
	};
	Instance->RootSignatureData->Bind(hScreen1, _countof(hScreen1));
	Instance->Screen->Draw();
	Instance->GaussRTVs[xIdx]->RTV2SRV();

	// ビューポート設定
	UINT yIdx = GaussRTVType::YBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	SetViewPort(Instance->GaussRTVs[yIdx]->Width, Instance->GaussRTVs[yIdx]->Height);

	// リソース設定
	Instance->GaussRTVs[yIdx]->SRV2RTV();
	Instance->GaussRTVs[yIdx]->Clear();

	// RTV設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs2[] = {
		Instance->GaussRTVs[yIdx]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs2);

	// YBlur
	Instance->PipelineData[GaussPipelineType::YBlurPipeline]->Bind();
	ID3D12DescriptorHeap* heaps2[] =
	{
		Instance->Heap->Get(),
	};
	DescriptorHeap::Bind(heaps2, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen2[] = {
		Instance->Params[gaussIdx * 2 + 1]->GetHandle().hGPU,
		Instance->GaussRTVs[xIdx]->GetHandleSRV().hGPU,
		Instance->GaussParam->GetHandle().hGPU,
	};
	Instance->RootSignatureData->Bind(hScreen2, _countof(hScreen2));
	Instance->Screen->Draw();
	Instance->GaussRTVs[yIdx]->RTV2SRV();

	// コピー
	Copy::ExecuteCopy(Instance->Heap.get(),
		Instance->GaussRTVs[yIdx]->GetHandleSRV().hGPU, dest);
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