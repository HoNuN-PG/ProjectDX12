
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Volume/Blur.h"
#include "System/Rendering/Volume/Copy.h"
#include "System/Rendering/Volume/volume.h"
#include "System/Rendering/RenderingEngine.h"
#include "System/DirectX.h"

namespace
{
	// カーネルサイズごとの設定
	struct KernelSetting
	{
		UINT			weightNum;	// 重みの数
		float			blur;		// ガウス関数の分散
		const wchar_t*	vsX;
		const wchar_t*	vsY;
		const wchar_t*	ps;
	};
	constexpr KernelSetting KernelSettings[] =
	{
		{ 2, 10.0f, L"../game/assets/shader/VS_Blur2X.cso", L"../game/assets/shader/VS_Blur2Y.cso", L"../game/assets/shader/PS_Blur2.cso" },
		{ 4, 10.0f, L"../game/assets/shader/VS_Blur4X.cso", L"../game/assets/shader/VS_Blur4Y.cso", L"../game/assets/shader/PS_Blur4.cso" },
		{ 8,  1.0f, L"../game/assets/shader/VS_Blur8X.cso", L"../game/assets/shader/VS_Blur8Y.cso", L"../game/assets/shader/PS_Blur8.cso" },
	};
	static_assert(_countof(KernelSettings) == static_cast<size_t>(Gauss::KernelType::MAX));
	
	// 重みの最大数
	constexpr UINT GAUSS_WEIGHTS_MAX = 8;
}

Gauss::Gauss(Copy& copy, MeshBuffer& screen)
	: pCopy(&copy)
	, pScreen(&screen)
{
	// ボリュームディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num =
			BlurParam::GAUSS_MAX +									// 入力SRV分確保
			BlurParam::GAUSS_MAX * GaussRTVType::RTV_MAX +			// RTV数分確保
			BlurParam::GAUSS_MAX * GaussScreenType::Screen_MAX +	// スクリーン定数バッファ分確保
			static_cast<UINT>(KernelType::MAX);						// 重み定数バッファ分確保
		pHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ボリュームディスクリプターヒープ(レンダーターゲット)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = BlurParam::GAUSS_MAX * GaussRTVType::RTV_MAX; // RTV数分確保
		pRTVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ルートシグネチャ（全カーネル共通）
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
		RootSignatureData = std::make_unique<RootSignature>(rootsignature);
	}
	// カーネルサイズごとのパイプラインと重み
	std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	PipelineState::Description pipeline = {
		L"",
		L"",
		L"",
		L"",
		RootSignatureData->Get(),
		PipelineState::IED_POS_TEX,
		PipelineState::IED_POS_TEX_COUNT,
		D3D12_CULL_MODE_BACK,
		formats,
		FALSE,
	};
	pipeline.Blend = PipelineState::BlendMode::None;

	for (int i = 0; i < static_cast<int>(KernelType::MAX); ++i)
	{
		const KernelSetting& setting = KernelSettings[i];
		KernelData& kernel = Kernels[i];

		// パイプライン
		pipeline.VSFile = setting.vsX;
		pipeline.PSFile = setting.ps;
		kernel.Pipeline[GaussPipelineType::XBlurPipeline] = std::make_unique<PipelineState>(pipeline);
		pipeline.VSFile = setting.vsY;
		pipeline.PSFile = setting.ps;
		kernel.Pipeline[GaussPipelineType::YBlurPipeline] = std::make_unique<PipelineState>(pipeline);

		// 重みの定数バッファ
		ConstantBuffer::Description desc = {};
		desc.pHeap = pHeap.get();
		desc.size = sizeof(float) * setting.weightNum;
		kernel.Weights = std::make_unique<ConstantBuffer>(desc);
		float weights[GAUSS_WEIGHTS_MAX] = {};
		CalcWeights(std::span<float>(weights, setting.weightNum), setting.blur);
		kernel.Weights->Write(weights);
	}
}

UINT32 Gauss::MakeGaussData(DirectX::XMFLOAT2 screen, KernelType kernel, UINT downSample)
{
	assert(Slots.size() < BlurParam::GAUSS_MAX);
	assert(kernel < KernelType::MAX);
	assert(downSample > 0);

	UINT32 gaussIdx = static_cast<UINT32>(Slots.size());
	Slot& slot = Slots.emplace_back();
	slot.KernelType = kernel;
	slot.SrcSRV = pHeap->Allocate(); // 入力SRV（実行時に入力テクスチャのSRVをコピーする）

	// Xブラーは横方向だけ縮小/Yブラーで縦方向も縮小する
	const float width = screen.x / downSample;
	const float height = screen.y / downSample;
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.pRTVHeap = pRTVHeap.get();
		desc.pSRVHeap = pHeap.get();

		desc.width = static_cast<UINT>(width);
		desc.height = static_cast<UINT>(screen.y);
		slot.RTVs[GaussRTVType::XBlur] = std::make_unique<RenderTarget>(desc);
		desc.width = static_cast<UINT>(width);
		desc.height = static_cast<UINT>(height);
		slot.RTVs[GaussRTVType::YBlur] = std::make_unique<RenderTarget>(desc);
	}
	// ConstantBuffer
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = pHeap.get();
		desc.size = sizeof(BlurParam::ScreenParam);
		slot.Screens[GaussScreenType::ScreenX] = std::make_unique<ConstantBuffer>(desc);
		slot.Screens[GaussScreenType::ScreenY] = std::make_unique<ConstantBuffer>(desc);

		// スクリーンサイズ
		BlurParam::ScreenParam x(width, screen.y, 1);
		BlurParam::ScreenParam y(width, height, 1);
		slot.Screens[GaussScreenType::ScreenX]->Write(&x);
		slot.Screens[GaussScreenType::ScreenY]->Write(&y);
	}

	return gaussIdx;
}

void Gauss::Execute(UINT32 gaussIdx, RenderTarget* src, RenderTarget* dest)
{
	assert(gaussIdx < Slots.size());
	Slot& slot = Slots[gaussIdx];
	KernelData& kernel = Kernels[static_cast<int>(slot.KernelType)];
	RenderTarget* xBlur = slot.RTVs[GaussRTVType::XBlur].get();
	RenderTarget* yBlur = slot.RTVs[GaussRTVType::YBlur].get();

	// XBlur
	RenderingEngine::CopyTexture(src->GetHandleSRV().hCPU, slot.SrcSRV.hCPU);
	DrawBlur(
		xBlur,
		slot.Screens[GaussScreenType::ScreenX]->GetHandle().hGPU,
		slot.SrcSRV.hGPU,
		kernel.Weights->GetHandle().hGPU,
		kernel.Pipeline[GaussPipelineType::XBlurPipeline].get()
	);

	// YBlur
	DrawBlur(
		yBlur,
		slot.Screens[GaussScreenType::ScreenY]->GetHandle().hGPU,
		xBlur->GetHandleSRV().hGPU,
		kernel.Weights->GetHandle().hGPU,
		kernel.Pipeline[GaussPipelineType::YBlurPipeline].get()
	);

	// コピー
	pCopy->ExecuteCopy(pHeap.get(), yBlur->GetHandleSRV().hGPU, dest);
}

void Gauss::CalcWeights(std::span<float> weights, float blur)
{
	// 重みの合計
	float total = 0;

	// ここからガウス関数を用いて重みを計算している
	// ループ変数のxが基準テクセルからの距離
	for (int x = 0; x < static_cast<int>(weights.size()); x++)
	{
		weights[x] = expf((float)(-x * x) / (2 * blur * blur));
		total += 2.0f * weights[x];
	}

	// 重みの合計で除算することで、重みの合計を1にしている
	for (float& w : weights)
	{
		w /= total;
	}
}

void Gauss::DrawBlur(
	RenderTarget* target,
	D3D12_GPU_DESCRIPTOR_HANDLE screen,
	D3D12_GPU_DESCRIPTOR_HANDLE src,
	D3D12_GPU_DESCRIPTOR_HANDLE weights,
	PipelineState* pipeline)
{
	// ビューポート設定
	SetViewPort(target->Width, target->Height);
	
	std::vector<RenderTarget*> targets = { target };
	D3D12_GPU_DESCRIPTOR_HANDLE handle[] =
	{
		screen,
		src,
		weights,
	};
	ScreenDraw(
		targets,
		pHeap->Get(),
		handle,
		_countof(handle),
		RootSignatureData.get(),
		pipeline,
		pScreen
	);
}
