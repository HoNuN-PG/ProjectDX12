#ifndef ___BLUR_H___
#define ___BLUR_H___

#include <memory>
#include <span>
#include <vector>

#include "Model/MeshBuffer.h"
#include "System/Rendering/ConstantBuffer/ConstantBuffer.h"
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Pipeline/RootSignature.h"
#include "System/Rendering/Pipeline/PipelineState.h"
#include "System/Rendering/Texture/RenderTarget.h"

namespace BlurParam
{
	// ブラースクリーン情報
	struct ScreenParam
	{
	public:
		float width;
		float height;
		float uvScale;
		float pad1;

	public:
		ScreenParam(float w, float h,float scale)
			:
			pad1(0)
		{
			width = w;
			height = h;
			uvScale = scale;
		}
	};

	// ガウスを実行できる最大数
	enum { GAUSS_MAX = 64 };
}

class Gauss
{
public:

	enum GaussPipelineType
	{
		XBlurPipeline = 0,
		YBlurPipeline,

		Pipeline_MAX
	};
	enum GaussRTVType
	{
		XBlur = 0,
		YBlur,

		RTV_MAX
	};
	enum GaussScreenType
	{
		ScreenX = 0,
		ScreenY,

		Screen_MAX
	};

	/// <summary>
	/// カーネルサイズ（片側のサンプル数）
	/// </summary>
	enum KernelType
	{
		Tap2 = 0,
		Tap4,
		Tap8,

		MAX
	};
	
public:

	Gauss(class Copy& copy, MeshBuffer& screen);
	~Gauss() {};

public:

	/// <summary>
	/// ガウスのデータ構築
	/// </summary>
	/// <param name="screen">入力テクスチャのサイズ</param>
	/// <param name="kernel">カーネルサイズ</param>
	/// <param name="downSample">ダウンサンプリングの強さ（1,2,4）</param>
	/// <returns>ガウススロット</returns>
	UINT32 MakeGaussData(DirectX::XMFLOAT2 screen, KernelType kernel, UINT downSample);

	/// <summary>
	/// ガウスの実行（カーネルサイズとダウンサンプリングはMakeGaussDataで指定したものを使う）
	/// </summary>
	/// <param name="gaussIdx">ガウススロット</param>
	/// <param name="src">入力テクスチャ</param>
	/// <param name="dest">出力先</param>
	void Execute(UINT32 gaussIdx, RenderTarget* src, RenderTarget* dest);

	/// <summary>
	/// ガウスの重みを計算（合計が1になるように正規化する）
	/// </summary>
	/// <param name="weights">書き込み先（要素数がサンプル数）</param>
	/// <param name="blur">ガウス関数の分散</param>
	static void CalcWeights(std::span<float> weights, float blur);

private:

	void DrawBlur(
		RenderTarget* target,
		D3D12_GPU_DESCRIPTOR_HANDLE screen,
		D3D12_GPU_DESCRIPTOR_HANDLE src,
		D3D12_GPU_DESCRIPTOR_HANDLE weights,
		PipelineState* pipeline
	);

private:

	// カーネルサイズごとのパイプラインと重み
	struct KernelData
	{
		std::unique_ptr<PipelineState>		Pipeline[GaussPipelineType::Pipeline_MAX];
		std::unique_ptr<ConstantBuffer>		Weights;
	};

	// ガウス1回分のデータ
	struct Slot
	{
		KernelType							KernelType;
		DescriptorHeap::Handle				SrcSRV;	// 入力テクスチャのSRVのコピー先
		std::unique_ptr<RenderTarget>		RTVs[GaussRTVType::RTV_MAX];
		std::unique_ptr<ConstantBuffer>		Screens[GaussScreenType::Screen_MAX];
	};
	
private:

	class Copy*							pCopy; // コピー処理（RenderingEngineが所有しGaussより長生きする）
	MeshBuffer*							pScreen; // 全画面描画用の四角形（RenderingEngineが所有しGaussより長生きする）
	std::unique_ptr<DescriptorHeap>		pHeap;
	std::unique_ptr<DescriptorHeap>		pRTVHeap;
	std::unique_ptr<RootSignature>		RootSignatureData; // 全カーネル共通
	KernelData							Kernels[static_cast<int>(KernelType::MAX)];
	std::vector<Slot>					Slots;

};

#endif
