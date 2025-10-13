#ifndef ___BLUR_H___
#define ___BLUR_H___

#include <memory>
#include <vector>

#include "MeshBuffer.h"

#include "ConstantBuffer.h"

#include "Pipeline.h"
#include "RootSignature.h"

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
			:pad1(0)
		{
			width = w;
			height = h;
			uvScale = scale;
		}
	};

	enum{GAUSS_WEIGHTS = 8};
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
	enum GaussParamsType
	{
		ScreenX = 0,
		ScreenY,

		Params_MAX
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
	/// <summary>
	/// ガウスの実行
	/// </summary>
	/// <param name="gaussIdx">ガウススロット：初期値として-1を与えると対応するスロット番号を取得できる</param>
	/// <param name="screen">テクスチャサイズ</param>
	/// <param name="src"></param>
	/// <param name="dest"></param>
	static void ExecuteScreenGauss2(int& gaussIdx,DirectX::XMFLOAT2 screen,
		std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void CalcWeights(std::weak_ptr<float[]> weights, float blur);

private:
	std::shared_ptr<DescriptorHeap>								Heap;
	std::shared_ptr<DescriptorHeap>								RTVHeap;
	std::unique_ptr<MeshBuffer>									Screen;
	std::unique_ptr<RootSignature>								RootSignatureData;
	std::vector<std::unique_ptr<Pipeline>>						PipelineData;
	std::vector<std::unique_ptr<RenderTarget>>					GaussRTVs;
	std::vector<std::unique_ptr<class ConstantBuffer>>			Params;
	std::unique_ptr<ConstantBuffer>								GaussParam;
	BlurParam::GaussParam										Weights;

};

#endif