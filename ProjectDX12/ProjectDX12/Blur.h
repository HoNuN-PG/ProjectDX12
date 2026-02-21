#ifndef ___BLUR_H___
#define ___BLUR_H___

#include <memory>
#include <vector>

// Model
#include "MeshBuffer.h"

// System/Constant
#include "ConstantBuffer.h"
// System/Rendering/Pipeline
#include "RootSignature.h"
#include "Pipeline.h"
// System/Rendering/Texture
#include "RenderTarget.h"

class DescriptorHeap;

namespace BlurParam
{
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
	struct Vertex
	{
		float pos[3];
		float uv[2];
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

	/// <summary>
	/// ガウスの実行
	/// </summary>
	/// <param name="gaussIdx">ガウススロット：-1を与えると対応するスロット番号を取得できる</param>
	/// <param name="screen">テクスチャサイズ</param>
	/// <param name="src"></param>
	/// <param name="dest"></param>
	static void ExecuteScreenGauss2D1(int& gaussIdx,DirectX::XMFLOAT2 screen,
	std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void ExecuteScreenGauss4D1(int& gaussIdx,DirectX::XMFLOAT2 screen,
	std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void ExecuteScreenGauss4D2(int& gaussIdx, DirectX::XMFLOAT2 screen,
		std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void ExecuteScreenGauss8D1(int& gaussIdx,DirectX::XMFLOAT2 screen,
	std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void ExecuteScreenGauss8D2(int& gaussIdx,DirectX::XMFLOAT2 screen,
		std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void ExecuteScreenGauss8D4(int& gaussIdx, DirectX::XMFLOAT2 screen,
		std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void CalcWeights(std::weak_ptr<float[]> weights,int num, float blur);

private:

	void ExecuteScreenGauss2(int& gaussIdx, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	void ExecuteScreenGauss4(int& gaussIdx, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	void ExecuteScreenGauss8(int& gaussIdx, std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	void MakeGaussData(int& gaussIdx, DirectX::XMFLOAT2 screen ,int split);

private:

	std::unique_ptr<MeshBuffer>									pScreen;
	std::shared_ptr<DescriptorHeap>								pHeap;
	std::shared_ptr<DescriptorHeap>								pRTVHeap;
	std::vector<std::unique_ptr<RenderTarget>>					GaussRTVs;
	std::vector<std::unique_ptr<class ConstantBuffer>>			Params;

private:

	// Gauss2
	std::unique_ptr<RootSignature>								Gauss2RootSignatureData;
	std::vector<std::unique_ptr<Pipeline>>						Gauss2PipelineData;
	// Gauss4
	std::unique_ptr<RootSignature>								Gauss4RootSignatureData;
	std::vector<std::unique_ptr<Pipeline>>						Gauss4PipelineData;
	// Gauss8
	std::unique_ptr<RootSignature>								Gauss8RootSignatureData;
	std::vector<std::unique_ptr<Pipeline>>						Gauss8PipelineData;

	std::unique_ptr<ConstantBuffer>								Gauss2Param;
	std::unique_ptr<ConstantBuffer>								Gauss4Param;
	std::unique_ptr<ConstantBuffer>								Gauss8Param;
	BlurParam::GaussParam										Weights2;
	BlurParam::GaussParam										Weights4;
	BlurParam::GaussParam										Weights8;

};

#endif