
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Volume/Blur.h"
#include "System/Rendering/Volume/Copy.h"
#include "System/Rendering/Volume/volume.h"
#include "System/Rendering/RenderingEngine.h"

std::unique_ptr<Gauss> Gauss::Instance;

void Gauss::Create()
{
	if (!Instance)
	{
		Instance = std::make_unique<Gauss>();

		// スクリーン頂点
		ScreenVertex screenVtx[] =
		{
			{{-0.5f, 0.5f,0} ,{0,0}} ,
			{{ 0.5f, 0.5f,0} ,{1,0}} ,
			{{-0.5f,-0.5f,0} ,{0,1}} ,
			{{ 0.5f,-0.5f,0} ,{1,1}} ,
		};

		// スクリーン
		MeshBuffer::Description desc = {};
		desc.pVtx = screenVtx;
		desc.vtxSize = sizeof(ScreenVertex);
		desc.vtxCount = _countof(screenVtx);
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		Instance->pScreen = std::make_unique<MeshBuffer>(desc);

		// ボリュームディスクリプタヒープ
		{
			DescriptorHeap::Description desc = {};
			desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.num = 
				BlurParam::GAUSS_MAX * GaussRTVType::RTV_MAX +			// RTV数分確保
				BlurParam::GAUSS_MAX * GaussScreenType::Screen_MAX +	// スクリーン定数バッファ分確保
				BlurParam::GAUSS_WEIGHTS_TYPE;							// 重み定数バッファ分確保
			Instance->pHeap = std::make_shared<DescriptorHeap>(desc);
		}
		// ボリュームディスクリプターヒープ(レンダーターゲット)
		{
			DescriptorHeap::Description desc = {};
			desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.num = 
				BlurParam::GAUSS_MAX * GaussRTVType::RTV_MAX;			// RTV数分確保
			Instance->pRTVHeap = std::make_shared<DescriptorHeap>(desc);
		}

		// Gauss2-----------------------------------------------------------------------
		// ルートシグネチャ
		{
			RootSignature::Parameter param[] = 
			{
				{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
				{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
				{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			};
			RootSignature::Description desc = {};
			desc.pParam = param;
			desc.paramNum = _countof(param);
			Instance->Gauss2RootSignatureData = std::make_unique<RootSignature>(desc);
		}
		// パイプライン
		{
			Instance->Gauss2PipelineData.resize(GaussPipelineType::Pipeline_MAX);

			PipelineState::Description desc = {};
			desc.pRootSignature = Instance->Gauss2RootSignatureData->Get();
			desc.pInputLayout = PipelineState::IED_POS_TEX;
			desc.InputLayoutNum = PipelineState::IED_POS_TEX_COUNT;
			desc.CullMode = D3D12_CULL_MODE_BACK;
			desc.RenderTargetNum = 1;
			desc.RenderTargetFormat.push_back(DXGI_FORMAT_R16G16B16A16_FLOAT);
			desc.WriteDepth = FALSE;

			desc.VSFile = L"../game/assets/shader/VS_Blur2X.cso";
			desc.PSFile = L"../game/assets/shader/PS_Blur2.cso";
			Instance->Gauss2PipelineData[GaussPipelineType::XBlurPipeline] = std::make_unique<PipelineState>(desc);

			desc.VSFile = L"../game/assets/shader/VS_Blur2Y.cso";
			desc.PSFile = L"../game/assets/shader/PS_Blur2.cso";
			Instance->Gauss2PipelineData[GaussPipelineType::YBlurPipeline] = std::make_unique<PipelineState>(desc);
		}
		// パラメーター定数バッファ
		{
			ConstantBuffer::Description desc = {};
			desc.pHeap = Instance->pHeap.get();
			desc.size = sizeof(float) * BlurParam::GAUSS2_WEIGHTS;
			Instance->Gauss2Param = std::make_unique<ConstantBuffer>(desc);

			// GaussWeights
			Instance->Weights2.weights = std::make_shared<float[]>(BlurParam::GAUSS2_WEIGHTS);
			CalcWeights(Instance->Weights2.weights, BlurParam::GAUSS2_WEIGHTS, 10);
			// 重みの設定
			{
				float w[BlurParam::GAUSS2_WEIGHTS];
				for (int i = 0; i < BlurParam::GAUSS2_WEIGHTS; ++i) { w[i] = Instance->Weights2.weights[i]; }
				Instance->Gauss2Param->Write(&w);
			}
		}

		// Gauss4-----------------------------------------------------------------------
		// ルートシグネチャ
		{
			RootSignature::Parameter param[] = 
			{
				{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
				{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
				{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			};
			RootSignature::Description desc = {};
			desc.pParam = param;
			desc.paramNum = _countof(param);
			Instance->Gauss4RootSignatureData = std::make_unique<RootSignature>(desc);
		}
		// パイプライン
		{
			Instance->Gauss4PipelineData.resize(GaussPipelineType::Pipeline_MAX);

			PipelineState::Description desc = {};
			desc.pRootSignature = Instance->Gauss4RootSignatureData->Get();
			desc.pInputLayout = PipelineState::IED_POS_TEX;
			desc.InputLayoutNum = PipelineState::IED_POS_TEX_COUNT;
			desc.CullMode = D3D12_CULL_MODE_BACK;
			desc.RenderTargetNum = 1;
			desc.RenderTargetFormat.push_back(DXGI_FORMAT_R16G16B16A16_FLOAT);
			desc.WriteDepth = FALSE;

			desc.VSFile = L"../game/assets/shader/VS_Blur4X.cso";
			desc.PSFile = L"../game/assets/shader/PS_Blur4.cso";
			Instance->Gauss4PipelineData[GaussPipelineType::XBlurPipeline] = std::make_unique<PipelineState>(desc);

			desc.VSFile = L"../game/assets/shader/VS_Blur4Y.cso";
			desc.PSFile = L"../game/assets/shader/PS_Blur4.cso";
			Instance->Gauss4PipelineData[GaussPipelineType::YBlurPipeline] = std::make_unique<PipelineState>(desc);
		}
		// パラメーター定数バッファ
		{
			ConstantBuffer::Description desc = {};
			desc.pHeap = Instance->pHeap.get();
			desc.size = sizeof(float) * BlurParam::GAUSS4_WEIGHTS;
			Instance->Gauss4Param = std::make_unique<ConstantBuffer>(desc);

			// GaussWeights
			Instance->Weights4.weights = std::make_shared<float[]>(BlurParam::GAUSS4_WEIGHTS);
			CalcWeights(Instance->Weights4.weights, BlurParam::GAUSS4_WEIGHTS, 10);
			// 重みの設定
			{
				float w[BlurParam::GAUSS4_WEIGHTS];
				for (int i = 0; i < BlurParam::GAUSS4_WEIGHTS; ++i) { w[i] = Instance->Weights4.weights[i]; }
				Instance->Gauss4Param->Write(&w);
			}
		}

		// Gauss8-----------------------------------------------------------------------
		// ルートシグネチャ
		{
			RootSignature::Parameter param[] = 
			{
				{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
				{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
				{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			};
			RootSignature::Description desc = {};
			desc.pParam = param;
			desc.paramNum = _countof(param);
			Instance->Gauss8RootSignatureData = std::make_unique<RootSignature>(desc);
		}
		// パイプライン
		{
			Instance->Gauss8PipelineData.resize(GaussPipelineType::Pipeline_MAX);

			PipelineState::Description desc = {};
			desc.pRootSignature = Instance->Gauss8RootSignatureData->Get();
			desc.pInputLayout = PipelineState::IED_POS_TEX;
			desc.InputLayoutNum = PipelineState::IED_POS_TEX_COUNT;
			desc.CullMode = D3D12_CULL_MODE_BACK;
			desc.RenderTargetNum = 1;
			desc.RenderTargetFormat.push_back(DXGI_FORMAT_R16G16B16A16_FLOAT);
			desc.WriteDepth = FALSE;

			desc.VSFile = L"../game/assets/shader/VS_Blur8X.cso";
			desc.PSFile = L"../game/assets/shader/PS_Blur8.cso";
			Instance->Gauss8PipelineData[GaussPipelineType::XBlurPipeline] = std::make_unique<PipelineState>(desc);

			desc.VSFile = L"../game/assets/shader/VS_Blur8Y.cso";
			desc.PSFile = L"../game/assets/shader/PS_Blur8.cso";
			Instance->Gauss8PipelineData[GaussPipelineType::YBlurPipeline] = std::make_unique<PipelineState>(desc);
		}
		// パラメーター定数バッファ
		{
			ConstantBuffer::Description desc = {};
			desc.pHeap = Instance->pHeap.get();
			desc.size = sizeof(float) * BlurParam::GAUSS8_WEIGHTS;
			Instance->Gauss8Param = std::make_unique<ConstantBuffer>(desc);

			// GaussWeights
			Instance->Weights8.weights = std::make_shared<float[]>(BlurParam::GAUSS8_WEIGHTS);
			CalcWeights(Instance->Weights8.weights, BlurParam::GAUSS8_WEIGHTS, 1);
			// 重みの設定
			{
				float w[BlurParam::GAUSS8_WEIGHTS];
				for (int i = 0; i < BlurParam::GAUSS8_WEIGHTS; ++i) { w[i] = Instance->Weights8.weights[i]; }
				Instance->Gauss8Param->Write(&w);
			}
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

UINT32 Gauss::MakeGaussD1Data(DirectX::XMFLOAT2 screen)
{
	UINT32 gaussIdx = Instance->GaussRTVs.size() / GaussRTVType::RTV_MAX;
	Instance->MakeGaussData(gaussIdx, screen, 1);
	return gaussIdx;
}

UINT32 Gauss::MakeGaussD2Data(DirectX::XMFLOAT2 screen)
{
	UINT32 gaussIdx = Instance->GaussRTVs.size() / GaussRTVType::RTV_MAX;
	Instance->MakeGaussData(gaussIdx, screen, 2);
	return gaussIdx;
}

UINT32 Gauss::MakeGaussD4Data(DirectX::XMFLOAT2 screen)
{
	UINT32 gaussIdx = Instance->GaussRTVs.size() / GaussRTVType::RTV_MAX;
	Instance->MakeGaussData(gaussIdx, screen, 4);
	return gaussIdx;
}

void Gauss::ExecuteScreenGauss2D1(UINT32 gaussIdx, DirectX::XMFLOAT2 screen, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest)
{
	Instance->ExecuteScreenGauss2(gaussIdx, src, dest);
}

void Gauss::ExecuteScreenGauss4D1(UINT32 gaussIdx, DirectX::XMFLOAT2 screen, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest)
{
	Instance->ExecuteScreenGauss4(gaussIdx, src, dest);
}

void Gauss::ExecuteScreenGauss4D2(UINT32 gaussIdx, DirectX::XMFLOAT2 screen, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest)
{
	Instance->ExecuteScreenGauss4(gaussIdx, src, dest);
}

void Gauss::ExecuteScreenGauss8D1(UINT32 gaussIdx, DirectX::XMFLOAT2 screen,
	std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest)
{
	Instance->ExecuteScreenGauss8(gaussIdx, src, dest);
}

void Gauss::ExecuteScreenGauss8D2(UINT32 gaussIdx, DirectX::XMFLOAT2 screen,
	std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest)
{
	Instance->ExecuteScreenGauss8(gaussIdx,src,dest);
}

void Gauss::ExecuteScreenGauss8D4(UINT32 gaussIdx, DirectX::XMFLOAT2 screen, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest)
{
	Instance->ExecuteScreenGauss8(gaussIdx, src, dest);
}

void Gauss::CalcWeights(std::weak_ptr<float[]> weights, int num, float blur)
{
	// 重みの合計
	float total = 0;

	// ここからガウス関数を用いて重みを計算している
	// ループ変数のxが基準テクセルからの距離
	for (int x = 0; x < num; x++)
	{
		weights.lock()[x] = expf((float)(-x * x) / (2 * blur * blur));
		total += 2.0f * weights.lock()[x];
	}

	// 重みの合計で除算することで、重みの合計を1にしている
	for (int i = 0; i < num; i++)
	{
		weights.lock()[i] /= total;
	}
}

void Gauss::ExecuteScreenGauss2(UINT32 gaussIdx, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest)
{
	//-------------------------------------------------------------------------------
	// ビューポート設定
	UINT xIdx = GaussRTVType::XBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	SetViewPort(Instance->GaussRTVs[xIdx]->Width, Instance->GaussRTVs[xIdx]->Height);

	//-------------------------------------------------------------------------------
	// リソース設定
	Instance->GaussRTVs[xIdx]->SRV2RTV();
	Instance->GaussRTVs[xIdx]->Clear();

	//-------------------------------------------------------------------------------
	// RTV設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs1[] = 
	{
		Instance->GaussRTVs[xIdx]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs1);

	//-------------------------------------------------------------------------------
	// XBlur
	UINT bIdx = GaussRTVType::Buffer + (gaussIdx * GaussRTVType::RTV_MAX);
	ID3D12DescriptorHeap* heaps1[] =
	{
		Instance->pHeap->Get(),
	};
	DescriptorHeap::Bind(heaps1, 1);
	RenderingEngine::CopyTextureSRV(src->GetHandleSRV().hCPU, Instance->GaussRTVs[bIdx]->GetHandleSRV().hCPU);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen1[] = 
	{
		Instance->Params[gaussIdx * 2]->GetHandle().hGPU,
		Instance->GaussRTVs[bIdx]->GetHandleSRV().hGPU,
		Instance->Gauss2Param->GetHandle().hGPU,
	};
	Instance->Gauss2RootSignatureData->Bind(hScreen1, _countof(hScreen1));
	Instance->Gauss2PipelineData[GaussPipelineType::XBlurPipeline]->Bind();
	Instance->pScreen->Draw();
	Instance->GaussRTVs[xIdx]->RTV2SRV();

	//-------------------------------------------------------------------------------
	// ビューポート設定
	UINT yIdx = GaussRTVType::YBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	SetViewPort(Instance->GaussRTVs[yIdx]->Width, Instance->GaussRTVs[yIdx]->Height);

	//-------------------------------------------------------------------------------
	// リソース設定
	Instance->GaussRTVs[yIdx]->SRV2RTV();
	Instance->GaussRTVs[yIdx]->Clear();

	//-------------------------------------------------------------------------------
	// RTV設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs2[] = 
	{
		Instance->GaussRTVs[yIdx]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs2);

	//-------------------------------------------------------------------------------
	// YBlur
	ID3D12DescriptorHeap* heaps2[] =
	{
		Instance->pHeap->Get(),
	};
	DescriptorHeap::Bind(heaps2, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen2[] = 
	{
		Instance->Params[gaussIdx * 2 + 1]->GetHandle().hGPU,
		Instance->GaussRTVs[xIdx]->GetHandleSRV().hGPU,
		Instance->Gauss2Param->GetHandle().hGPU,
	};
	Instance->Gauss2RootSignatureData->Bind(hScreen2, _countof(hScreen2));
	Instance->Gauss2PipelineData[GaussPipelineType::YBlurPipeline]->Bind();
	Instance->pScreen->Draw();
	Instance->GaussRTVs[yIdx]->RTV2SRV();

	//-------------------------------------------------------------------------------
	// コピー
	dest->SRV2RTV();
	Copy::ExecuteCopy(Instance->pHeap.get(), Instance->GaussRTVs[yIdx]->GetHandleSRV().hGPU, dest);
	dest->RTV2SRV();
}

void Gauss::ExecuteScreenGauss4(UINT32 gaussIdx, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest)
{
	//-------------------------------------------------------------------------------
	// ビューポート設定
	UINT xIdx = GaussRTVType::XBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	SetViewPort(Instance->GaussRTVs[xIdx]->Width, Instance->GaussRTVs[xIdx]->Height);

	//-------------------------------------------------------------------------------
	// リソース設定
	Instance->GaussRTVs[xIdx]->SRV2RTV();
	Instance->GaussRTVs[xIdx]->Clear();

	//-------------------------------------------------------------------------------
	// RTV設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs1[] = 
	{
		Instance->GaussRTVs[xIdx]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs1);

	//-------------------------------------------------------------------------------
	// XBlur
	UINT bIdx = GaussRTVType::Buffer + (gaussIdx * GaussRTVType::RTV_MAX);
	ID3D12DescriptorHeap* heaps1[] =
	{
		Instance->pHeap->Get(),
	};
	DescriptorHeap::Bind(heaps1, 1);
	RenderingEngine::CopyTextureSRV(src->GetHandleSRV().hCPU, Instance->GaussRTVs[bIdx]->GetHandleSRV().hCPU);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen1[] = 
	{
		Instance->Params[gaussIdx * 2]->GetHandle().hGPU,
		Instance->GaussRTVs[bIdx]->GetHandleSRV().hGPU,
		Instance->Gauss4Param->GetHandle().hGPU,
	};
	Instance->Gauss4RootSignatureData->Bind(hScreen1, _countof(hScreen1));
	Instance->Gauss4PipelineData[GaussPipelineType::XBlurPipeline]->Bind();
	Instance->pScreen->Draw();
	Instance->GaussRTVs[xIdx]->RTV2SRV();

	//-------------------------------------------------------------------------------
	// ビューポート設定
	UINT yIdx = GaussRTVType::YBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	SetViewPort(Instance->GaussRTVs[yIdx]->Width, Instance->GaussRTVs[yIdx]->Height);

	//-------------------------------------------------------------------------------
	// リソース設定
	Instance->GaussRTVs[yIdx]->SRV2RTV();
	Instance->GaussRTVs[yIdx]->Clear();

	//-------------------------------------------------------------------------------
	// RTV設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs2[] = 
	{
		Instance->GaussRTVs[yIdx]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs2);

	//-------------------------------------------------------------------------------
	// YBlur
	ID3D12DescriptorHeap* heaps2[] =
	{
		Instance->pHeap->Get(),
	};
	DescriptorHeap::Bind(heaps2, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen2[] = 
	{
		Instance->Params[gaussIdx * 2 + 1]->GetHandle().hGPU,
		Instance->GaussRTVs[xIdx]->GetHandleSRV().hGPU,
		Instance->Gauss4Param->GetHandle().hGPU,
	};
	Instance->Gauss4RootSignatureData->Bind(hScreen2, _countof(hScreen2));
	Instance->Gauss4PipelineData[GaussPipelineType::YBlurPipeline]->Bind();
	Instance->pScreen->Draw();
	Instance->GaussRTVs[yIdx]->RTV2SRV();

	//-------------------------------------------------------------------------------
	// コピー
	dest->SRV2RTV();
	Copy::ExecuteCopy(Instance->pHeap.get(), Instance->GaussRTVs[yIdx]->GetHandleSRV().hGPU, dest);
	dest->RTV2SRV();
}

void Gauss::ExecuteScreenGauss8(UINT32 gaussIdx, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest)
{
	//-------------------------------------------------------------------------------
	// ビューポート設定
	UINT xIdx = GaussRTVType::XBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	SetViewPort(Instance->GaussRTVs[xIdx]->Width, Instance->GaussRTVs[xIdx]->Height);

	//-------------------------------------------------------------------------------
	// リソース設定
	Instance->GaussRTVs[xIdx]->SRV2RTV();
	Instance->GaussRTVs[xIdx]->Clear();

	//-------------------------------------------------------------------------------
	// RTV設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs1[] = 
	{
		Instance->GaussRTVs[xIdx]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs1);

	//-------------------------------------------------------------------------------
	// XBlur
	UINT bIdx = GaussRTVType::Buffer + (gaussIdx * GaussRTVType::RTV_MAX);
	ID3D12DescriptorHeap* heaps1[] =
	{
		Instance->pHeap->Get(),
	};
	DescriptorHeap::Bind(heaps1, 1);
	RenderingEngine::CopyTextureSRV(src->GetHandleSRV().hCPU, Instance->GaussRTVs[bIdx]->GetHandleSRV().hCPU);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen1[] = 
	{
		Instance->Params[gaussIdx * 2]->GetHandle().hGPU,
		Instance->GaussRTVs[bIdx]->GetHandleSRV().hGPU,
		Instance->Gauss8Param->GetHandle().hGPU,
	};
	Instance->Gauss8RootSignatureData->Bind(hScreen1, _countof(hScreen1));
	Instance->Gauss8PipelineData[GaussPipelineType::XBlurPipeline]->Bind();
	Instance->pScreen->Draw();
	Instance->GaussRTVs[xIdx]->RTV2SRV();

	//-------------------------------------------------------------------------------
	// ビューポート設定
	UINT yIdx = GaussRTVType::YBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	SetViewPort(Instance->GaussRTVs[yIdx]->Width, Instance->GaussRTVs[yIdx]->Height);

	//-------------------------------------------------------------------------------
	// リソース設定
	Instance->GaussRTVs[yIdx]->SRV2RTV();
	Instance->GaussRTVs[yIdx]->Clear();

	//-------------------------------------------------------------------------------
	// RTV設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs2[] = 
	{
		Instance->GaussRTVs[yIdx]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs2);

	//-------------------------------------------------------------------------------
	// YBlur
	ID3D12DescriptorHeap* heaps2[] =
	{
		Instance->pHeap->Get(),
	};
	DescriptorHeap::Bind(heaps2, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen2[] = 
	{
		Instance->Params[gaussIdx * 2 + 1]->GetHandle().hGPU,
		Instance->GaussRTVs[xIdx]->GetHandleSRV().hGPU,
		Instance->Gauss8Param->GetHandle().hGPU,
	};
	Instance->Gauss8RootSignatureData->Bind(hScreen2, _countof(hScreen2));
	Instance->Gauss8PipelineData[GaussPipelineType::YBlurPipeline]->Bind();
	Instance->pScreen->Draw();
	Instance->GaussRTVs[yIdx]->RTV2SRV();

	//-------------------------------------------------------------------------------
	// コピー
	dest->SRV2RTV();
	Copy::ExecuteCopy(Instance->pHeap.get(), Instance->GaussRTVs[yIdx]->GetHandleSRV().hGPU, dest);
	dest->RTV2SRV();
}

void Gauss::MakeGaussData(UINT32 gaussIdx, DirectX::XMFLOAT2 screen, int split)
{
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.pRTVHeap = Instance->pRTVHeap.get();
		desc.pSRVHeap = Instance->pHeap.get();

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
		desc.pHeap = Instance->pHeap.get();
		desc.size = sizeof(BlurParam::ScreenParam);
		Instance->Params.push_back(std::make_unique<ConstantBuffer>(desc));
		Instance->Params.push_back(std::make_unique<ConstantBuffer>(desc));

		// スクリーンサイズ
		BlurParam::ScreenParam p1(screen.x / split, screen.y, 1);
		BlurParam::ScreenParam p2(screen.x / split, screen.y / split, 1);
		Instance->Params[gaussIdx * 2]->Write(&p1);
		Instance->Params[gaussIdx * 2 + 1]->Write(&p2);
	}
}