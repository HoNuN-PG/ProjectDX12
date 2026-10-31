
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Volume/Blur.h"
#include "System/Rendering/Volume/Copy.h"
#include "System/Rendering/Volume/volume.h"
#include "System/Rendering/RenderingEngine.h"
#include "System/DirectX.h"

Gauss::Gauss(Copy& copy)
	: pCopy(&copy)
{
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
	pScreen = std::make_unique<MeshBuffer>(desc);

	// ボリュームディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 
			BlurParam::GAUSS_MAX * GaussRTVType::RTV_MAX +			// RTV数分確保
			BlurParam::GAUSS_MAX +									// 入力SRV分確保
			BlurParam::GAUSS_MAX * GaussScreenType::Screen_MAX +	// スクリーン定数バッファ分確保
			BlurParam::GAUSS_WEIGHTS_TYPE;							// 重み定数バッファ分確保
		pHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ボリュームディスクリプターヒープ(レンダーターゲット)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = 
			BlurParam::GAUSS_MAX * GaussRTVType::RTV_MAX;			// RTV数分確保
		pRTVHeap = std::make_unique<DescriptorHeap>(desc);
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
		RootSignature::Description rootsignature =
		{
			param,
			_countof(param)
		};
		Gauss2RootSignatureData = std::make_unique<RootSignature>(rootsignature);
	}
	// パイプライン
	{
		Gauss2PipelineData.resize(GaussPipelineType::Pipeline_MAX);

		std::vector<DXGI_FORMAT> foarmats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
		PipelineState::Description desc = {
			L"",
			L"",
			L"",
			L"",
			Gauss2RootSignatureData->Get(),
			PipelineState::IED_POS_TEX,
			PipelineState::IED_POS_TEX_COUNT,
			D3D12_CULL_MODE_BACK,
			1,
			foarmats,
			FALSE,
		};

		desc.VSFile = L"../game/assets/shader/VS_Blur2X.cso";
		desc.PSFile = L"../game/assets/shader/PS_Blur2.cso";
		Gauss2PipelineData[GaussPipelineType::XBlurPipeline] = std::make_unique<PipelineState>(desc);

		desc.VSFile = L"../game/assets/shader/VS_Blur2Y.cso";
		desc.PSFile = L"../game/assets/shader/PS_Blur2.cso";
		Gauss2PipelineData[GaussPipelineType::YBlurPipeline] = std::make_unique<PipelineState>(desc);
	}
	// パラメーター定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = pHeap.get();
		desc.size = sizeof(float) * BlurParam::GAUSS2_WEIGHTS;
		Gauss2Param = std::make_unique<ConstantBuffer>(desc);

		// GaussWeights
		Weights2.weights = std::make_shared<float[]>(BlurParam::GAUSS2_WEIGHTS);
		CalcWeights(Weights2.weights, BlurParam::GAUSS2_WEIGHTS, 10);
		// 重みの設定
		{
			float w[BlurParam::GAUSS2_WEIGHTS];
			for (int i = 0; i < BlurParam::GAUSS2_WEIGHTS; ++i) { w[i] = Weights2.weights[i]; }
			Gauss2Param->Write(&w);
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
		RootSignature::Description rootsignature =
		{
			param,
			_countof(param)
		};
		Gauss4RootSignatureData = std::make_unique<RootSignature>(rootsignature);
	}
	// パイプライン
	{
		Gauss4PipelineData.resize(GaussPipelineType::Pipeline_MAX);

		std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
		PipelineState::Description desc = {
			L"",
			L"",
			L"",
			L"",
			Gauss4RootSignatureData->Get(),
			PipelineState::IED_POS_TEX,
			PipelineState::IED_POS_TEX_COUNT,
			D3D12_CULL_MODE_BACK,
			1,
			formats,
			FALSE,
		};

		desc.VSFile = L"../game/assets/shader/VS_Blur4X.cso";
		desc.PSFile = L"../game/assets/shader/PS_Blur4.cso";
		Gauss4PipelineData[GaussPipelineType::XBlurPipeline] = std::make_unique<PipelineState>(desc);

		desc.VSFile = L"../game/assets/shader/VS_Blur4Y.cso";
		desc.PSFile = L"../game/assets/shader/PS_Blur4.cso";
		Gauss4PipelineData[GaussPipelineType::YBlurPipeline] = std::make_unique<PipelineState>(desc);
	}
	// パラメーター定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = pHeap.get();
		desc.size = sizeof(float) * BlurParam::GAUSS4_WEIGHTS;
		Gauss4Param = std::make_unique<ConstantBuffer>(desc);

		// GaussWeights
		Weights4.weights = std::make_shared<float[]>(BlurParam::GAUSS4_WEIGHTS);
		CalcWeights(Weights4.weights, BlurParam::GAUSS4_WEIGHTS, 10);
		// 重みの設定
		{
			float w[BlurParam::GAUSS4_WEIGHTS];
			for (int i = 0; i < BlurParam::GAUSS4_WEIGHTS; ++i) { w[i] = Weights4.weights[i]; }
			Gauss4Param->Write(&w);
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
		RootSignature::Description rootsignature =
		{
			param,
			_countof(param)
		};
		Gauss8RootSignatureData = std::make_unique<RootSignature>(rootsignature);
	}
	// パイプライン
	{
		Gauss8PipelineData.resize(GaussPipelineType::Pipeline_MAX);

		std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
		PipelineState::Description desc = {
			L"",
			L"",
			L"",
			L"",
			Gauss8RootSignatureData->Get(),
			PipelineState::IED_POS_TEX,
			PipelineState::IED_POS_TEX_COUNT,
			D3D12_CULL_MODE_BACK,
			1,
			formats,
			FALSE,
		};

		desc.VSFile = L"../game/assets/shader/VS_Blur8X.cso";
		desc.PSFile = L"../game/assets/shader/PS_Blur8.cso";
		Gauss8PipelineData[GaussPipelineType::XBlurPipeline] = std::make_unique<PipelineState>(desc);

		desc.VSFile = L"../game/assets/shader/VS_Blur8Y.cso";
		desc.PSFile = L"../game/assets/shader/PS_Blur8.cso";
		Gauss8PipelineData[GaussPipelineType::YBlurPipeline] = std::make_unique<PipelineState>(desc);
	}
	// パラメーター定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = pHeap.get();
		desc.size = sizeof(float) * BlurParam::GAUSS8_WEIGHTS;
		Gauss8Param = std::make_unique<ConstantBuffer>(desc);

		// GaussWeights
		Weights8.weights = std::make_shared<float[]>(BlurParam::GAUSS8_WEIGHTS);
		CalcWeights(Weights8.weights, BlurParam::GAUSS8_WEIGHTS, 1);
		// 重みの設定
		{
			float w[BlurParam::GAUSS8_WEIGHTS];
			for (int i = 0; i < BlurParam::GAUSS8_WEIGHTS; ++i) { w[i] = Weights8.weights[i]; }
			Gauss8Param->Write(&w);
		}
	}
}

UINT32 Gauss::MakeGaussD1Data(DirectX::XMFLOAT2 screen)
{
	UINT32 gaussIdx = GaussRTVs.size() / GaussRTVType::RTV_MAX;
	MakeGaussData(gaussIdx, screen, 1);
	return gaussIdx;
}

UINT32 Gauss::MakeGaussD2Data(DirectX::XMFLOAT2 screen)
{
	UINT32 gaussIdx = GaussRTVs.size() / GaussRTVType::RTV_MAX;
	MakeGaussData(gaussIdx, screen, 2);
	return gaussIdx;
}

UINT32 Gauss::MakeGaussD4Data(DirectX::XMFLOAT2 screen)
{
	UINT32 gaussIdx = GaussRTVs.size() / GaussRTVType::RTV_MAX;
	MakeGaussData(gaussIdx, screen, 4);
	return gaussIdx;
}

void Gauss::ExecuteScreenGauss2D1(UINT32 gaussIdx, DirectX::XMFLOAT2 screen, RenderTarget* src, RenderTarget* dest)
{
	ExecuteScreenGauss2(gaussIdx, src, dest);
}

void Gauss::ExecuteScreenGauss4D1(UINT32 gaussIdx, DirectX::XMFLOAT2 screen, RenderTarget* src, RenderTarget* dest)
{
	ExecuteScreenGauss4(gaussIdx, src, dest);
}

void Gauss::ExecuteScreenGauss4D2(UINT32 gaussIdx, DirectX::XMFLOAT2 screen, RenderTarget* src, RenderTarget* dest)
{
	ExecuteScreenGauss4(gaussIdx, src, dest);
}

void Gauss::ExecuteScreenGauss8D1(UINT32 gaussIdx, DirectX::XMFLOAT2 screen,
	RenderTarget* src, RenderTarget* dest)
{
	ExecuteScreenGauss8(gaussIdx, src, dest);
}

void Gauss::ExecuteScreenGauss8D2(UINT32 gaussIdx, DirectX::XMFLOAT2 screen,
	RenderTarget* src, RenderTarget* dest)
{
	ExecuteScreenGauss8(gaussIdx,src,dest);
}

void Gauss::ExecuteScreenGauss8D4(UINT32 gaussIdx, DirectX::XMFLOAT2 screen, RenderTarget* src, RenderTarget* dest)
{
	ExecuteScreenGauss8(gaussIdx, src, dest);
}

void Gauss::CalcWeights(std::weak_ptr<float[]> weights, int num, float blur)
{
	// 重み配列は呼び出し元のGaussが保持しているため常に有効
	std::shared_ptr<float[]> w = weights.lock();
	assert(w);

	// 重みの合計
	float total = 0;

	// ここからガウス関数を用いて重みを計算している
	// ループ変数のxが基準テクセルからの距離
	for (int x = 0; x < num; x++)
	{
		w[x] = expf((float)(-x * x) / (2 * blur * blur));
		total += 2.0f * w[x];
	}

	// 重みの合計で除算することで、重みの合計を1にしている
	for (int i = 0; i < num; i++)
	{
		w[i] /= total;
	}
}

void Gauss::ExecuteScreenGauss2(UINT32 gaussIdx, RenderTarget* src, RenderTarget* dest)
{
	UINT xIdx = GaussRTVType::XBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	UINT yIdx = GaussRTVType::YBlur + (gaussIdx * GaussRTVType::RTV_MAX);

	//-------------------------------------------------------------------------------
	// ビューポート設定
	SetViewPort(GaussRTVs[xIdx]->Width, GaussRTVs[xIdx]->Height);

	//-------------------------------------------------------------------------------
	// XBlur
	RenderingEngine::CopyTexture(src->GetHandleSRV().hCPU, SrcSRVs[gaussIdx].hCPU);
	std::vector<RenderTarget*> targets1 = { GaussRTVs[xIdx].get()};
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen1[] =
	{
		Params[gaussIdx * 2]->GetHandle().hGPU,
		SrcSRVs[gaussIdx].hGPU,
		Gauss2Param->GetHandle().hGPU,
	};
	ScreenDraw(
		targets1, 
		pHeap->Get(), 
		hScreen1, 
		_countof(hScreen1), 
		Gauss2RootSignatureData.get(), 
		Gauss2PipelineData[GaussPipelineType::XBlurPipeline].get(), 
		pScreen.get()
	);

	//-------------------------------------------------------------------------------
	// ビューポート設定
	SetViewPort(GaussRTVs[yIdx]->Width, GaussRTVs[yIdx]->Height);

	//-------------------------------------------------------------------------------
	// YBlur
	std::vector<RenderTarget*> targets2 = { GaussRTVs[yIdx].get() };
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen2[] =
	{
		Params[gaussIdx * 2 + 1]->GetHandle().hGPU,
		GaussRTVs[xIdx]->GetHandleSRV().hGPU,
		Gauss2Param->GetHandle().hGPU,
	};
	ScreenDraw(
		targets2,
		pHeap->Get(),
		hScreen2,
		_countof(hScreen2),
		Gauss2RootSignatureData.get(),
		Gauss2PipelineData[GaussPipelineType::YBlurPipeline].get(),
		pScreen.get()
	);

	//-------------------------------------------------------------------------------
	// コピー
	pCopy->ExecuteCopy(pHeap.get(), GaussRTVs[yIdx]->GetHandleSRV().hGPU, dest);
}

void Gauss::ExecuteScreenGauss4(UINT32 gaussIdx, RenderTarget* src, RenderTarget* dest)
{
	UINT xIdx = GaussRTVType::XBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	UINT yIdx = GaussRTVType::YBlur + (gaussIdx * GaussRTVType::RTV_MAX);

	//-------------------------------------------------------------------------------
	// ビューポート設定
	SetViewPort(GaussRTVs[xIdx]->Width, GaussRTVs[xIdx]->Height);

	//-------------------------------------------------------------------------------
	// XBlur
	RenderingEngine::CopyTexture(src->GetHandleSRV().hCPU, SrcSRVs[gaussIdx].hCPU);
	std::vector<RenderTarget*> targets1 = { GaussRTVs[xIdx].get() };
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen1[] =
	{
		Params[gaussIdx * 2]->GetHandle().hGPU,
		SrcSRVs[gaussIdx].hGPU,
		Gauss4Param->GetHandle().hGPU,
	};
	ScreenDraw(
		targets1,
		pHeap->Get(),
		hScreen1,
		_countof(hScreen1),
		Gauss4RootSignatureData.get(),
		Gauss4PipelineData[GaussPipelineType::XBlurPipeline].get(),
		pScreen.get()
	);

	//-------------------------------------------------------------------------------
	// ビューポート設定
	SetViewPort(GaussRTVs[yIdx]->Width, GaussRTVs[yIdx]->Height);

	//-------------------------------------------------------------------------------
	// YBlur
	std::vector<RenderTarget*> targets2 = { GaussRTVs[yIdx].get() };
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen2[] =
	{
		Params[gaussIdx * 2 + 1]->GetHandle().hGPU,
		GaussRTVs[xIdx]->GetHandleSRV().hGPU,
		Gauss4Param->GetHandle().hGPU,
	};
	ScreenDraw(
		targets2,
		pHeap->Get(),
		hScreen2,
		_countof(hScreen2),
		Gauss4RootSignatureData.get(),
		Gauss4PipelineData[GaussPipelineType::YBlurPipeline].get(),
		pScreen.get()
	);

	//-------------------------------------------------------------------------------
	// コピー
	pCopy->ExecuteCopy(pHeap.get(), GaussRTVs[yIdx]->GetHandleSRV().hGPU, dest);
}

void Gauss::ExecuteScreenGauss8(UINT32 gaussIdx, RenderTarget* src, RenderTarget* dest)
{
	UINT xIdx = GaussRTVType::XBlur + (gaussIdx * GaussRTVType::RTV_MAX);
	UINT yIdx = GaussRTVType::YBlur + (gaussIdx * GaussRTVType::RTV_MAX);

	//-------------------------------------------------------------------------------
	// ビューポート設定
	SetViewPort(GaussRTVs[xIdx]->Width, GaussRTVs[xIdx]->Height);

	//-------------------------------------------------------------------------------
	// XBlur
	RenderingEngine::CopyTexture(src->GetHandleSRV().hCPU, SrcSRVs[gaussIdx].hCPU);
	std::vector<RenderTarget*> targets1 = { GaussRTVs[xIdx].get() };
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen1[] =
	{
		Params[gaussIdx * 2]->GetHandle().hGPU,
		SrcSRVs[gaussIdx].hGPU,
		Gauss8Param->GetHandle().hGPU,
	};
	ScreenDraw(
		targets1,
		pHeap->Get(),
		hScreen1,
		_countof(hScreen1),
		Gauss8RootSignatureData.get(),
		Gauss8PipelineData[GaussPipelineType::XBlurPipeline].get(),
		pScreen.get()
	);

	//-------------------------------------------------------------------------------
	// ビューポート設定
	SetViewPort(GaussRTVs[yIdx]->Width, GaussRTVs[yIdx]->Height);

	//-------------------------------------------------------------------------------
	// YBlur
	std::vector<RenderTarget*> targets2 = { GaussRTVs[yIdx].get() };
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen2[] =
	{
		Params[gaussIdx * 2 + 1]->GetHandle().hGPU,
		GaussRTVs[xIdx]->GetHandleSRV().hGPU,
		Gauss8Param->GetHandle().hGPU,
	};
	ScreenDraw(
		targets2,
		pHeap->Get(),
		hScreen2,
		_countof(hScreen2),
		Gauss8RootSignatureData.get(),
		Gauss8PipelineData[GaussPipelineType::YBlurPipeline].get(),
		pScreen.get()
	);

	//-------------------------------------------------------------------------------
	// コピー
	pCopy->ExecuteCopy(pHeap.get(), GaussRTVs[yIdx]->GetHandleSRV().hGPU, dest);
}

void Gauss::MakeGaussData(UINT32 gaussIdx, DirectX::XMFLOAT2 screen, int split)
{
	// 入力SRV（実行時に入力テクスチャのSRVをコピーする）
	SrcSRVs.push_back(pHeap->Allocate());

	// RTV
	{
		RenderTarget::Description desc = {};
		desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.pRTVHeap = pRTVHeap.get();
		desc.pSRVHeap = pHeap.get();

		desc.width = screen.x / split;
		desc.height = screen.y;
		GaussRTVs.push_back(std::make_unique<RenderTarget>(desc));

		desc.width = screen.x / split;
		desc.height = screen.y / split;
		GaussRTVs.push_back(std::make_unique<RenderTarget>(desc));
	}
	// ConstantBuffer
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = pHeap.get();
		desc.size = sizeof(BlurParam::ScreenParam);
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
		Params.push_back(std::make_unique<ConstantBuffer>(desc));

		// スクリーンサイズ
		BlurParam::ScreenParam p1(screen.x / split, screen.y, 1);
		BlurParam::ScreenParam p2(screen.x / split, screen.y / split, 1);
		Params[gaussIdx * 2]->Write(&p1);
		Params[gaussIdx * 2 + 1]->Write(&p2);
	}
}