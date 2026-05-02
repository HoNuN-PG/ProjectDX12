#ifndef ___BLUR_H___
#define ___BLUR_H___

#include <memory>
#include <vector>

#include "Model/MeshBuffer.h"
#include "System/Rendering/ConstantBuffer/ConstantBuffer.h"
#include "System/Rendering/Pipeline/RootSignature.h"
#include "System/Rendering/Pipeline/PipelineState.h"
#include "System/Rendering/Texture/RenderTarget.h"

class DescriptorHeap;

namespace BlurParam
{
	// ブラースクリーン情報
	struct ScreenParam
	{
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

	// ガウスの重さ
	enum{GAUSS_WEIGHTS_TYPE = 3};
	enum{GAUSS2_WEIGHTS = 2};
	enum{GAUSS4_WEIGHTS = 4};
	enum{GAUSS8_WEIGHTS = 8};
	struct GaussParam
	{
		std::shared_ptr<float[]> weights;
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
		Buffer = 0,
		XBlur,
		YBlur,

		RTV_MAX
	};
	enum GaussScreenType
	{
		ScreenX = 0,
		ScreenY,

		Screen_MAX
	};

public:

	static void Create();
	static void Destroy();

private:

	static std::unique_ptr<Gauss> Instance;

public:

	Gauss() {};
	~Gauss() {};

public:

	// ガウスのデータ構築（D1,D2,D4はダウンサンプリングの強さ）
	static UINT32 MakeGaussD1Data(DirectX::XMFLOAT2 screen);
	static UINT32 MakeGaussD2Data(DirectX::XMFLOAT2 screen);
	static UINT32 MakeGaussD4Data(DirectX::XMFLOAT2 screen);

public:

	/// <summary>
	/// ガウスの実行
	/// </summary>
	/// <param name="gaussIdx">ガウススロット</param>
	/// <param name="screen">テクスチャサイズ</param>
	/// <param name="src"></param>
	/// <param name="dest"></param>
	static void ExecuteScreenGauss2D1(UINT32 gaussIdx,DirectX::XMFLOAT2 screen,
	std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void ExecuteScreenGauss4D1(UINT32 gaussIdx,DirectX::XMFLOAT2 screen,
	std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void ExecuteScreenGauss4D2(UINT32 gaussIdx, DirectX::XMFLOAT2 screen,
		std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void ExecuteScreenGauss8D1(UINT32 gaussIdx,DirectX::XMFLOAT2 screen,
	std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void ExecuteScreenGauss8D2(UINT32 gaussIdx,DirectX::XMFLOAT2 screen,
		std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void ExecuteScreenGauss8D4(UINT32 gaussIdx, DirectX::XMFLOAT2 screen,
		std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void CalcWeights(std::weak_ptr<float[]> weights,int num, float blur);

private:

	void ExecuteScreenGauss2(UINT32 gaussIdx, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	void ExecuteScreenGauss4(UINT32 gaussIdx, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	void ExecuteScreenGauss8(UINT32 gaussIdx, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	void MakeGaussData(UINT32 gaussIdx, DirectX::XMFLOAT2 screen ,int split);

private:

	std::unique_ptr<MeshBuffer>									pScreen;
	std::shared_ptr<DescriptorHeap>								pHeap;
	std::shared_ptr<DescriptorHeap>								pRTVHeap;
	std::vector<std::unique_ptr<RenderTarget>>					GaussRTVs;
	std::vector<std::unique_ptr<class ConstantBuffer>>			Params;

private:

	// Gauss2
	std::unique_ptr<RootSignature>								Gauss2RootSignatureData;
	std::vector<std::unique_ptr<PipelineState>>					Gauss2PipelineData;
	// Gauss4
	std::unique_ptr<RootSignature>								Gauss4RootSignatureData;
	std::vector<std::unique_ptr<PipelineState>>					Gauss4PipelineData;
	// Gauss8
	std::unique_ptr<RootSignature>								Gauss8RootSignatureData;
	std::vector<std::unique_ptr<PipelineState>>					Gauss8PipelineData;

	std::unique_ptr<ConstantBuffer>								Gauss2Param;
	std::unique_ptr<ConstantBuffer>								Gauss4Param;
	std::unique_ptr<ConstantBuffer>								Gauss8Param;
	BlurParam::GaussParam										Weights2;
	BlurParam::GaussParam										Weights4;
	BlurParam::GaussParam										Weights8;

};

#endif