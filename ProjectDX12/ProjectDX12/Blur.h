#ifndef ___BLUR_H___
#define ___BLUR_H___

#include <memory>
#include <vector>

#include "MeshBuffer.h"
#include "Pipeline.h"
#include "RootSignature.h"

class DescriptorHeap;
class RenderTarget;

namespace BlurParam
{
	struct ScreenParam
	{
		float width;
		float height;
		DirectX::XMFLOAT2 pad1;

	public:
		ScreenParam(float w, float h)
			:pad1({0,0})
		{
			width = w;
			height = h;
		}
	};

	enum{GAUSS_WEIGHTS = 8};
	struct GaussParam
	{
		std::shared_ptr<float[]> weights;
	};
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
	enum GaussRTVsType
	{
		Buffer = 0,
		XBlur,
		YBlur,

		RTVs_MAX
	};
	enum GaussParamsType
	{
		ScreenX = 0,
		ScreenY,
		GaussWeights,

		Params_MAX
	};

public:
	struct Vertex
	{
		float pos[3];
		float uv[2];
	};

public:
	Gauss() {};
	~Gauss() {};
	static void Load();
	static void ExecuteGauss(std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest,
		std::weak_ptr<float[]> weights = std::weak_ptr<float[]>{});
	static void CalcWeights(std::weak_ptr<float[]> weights, float blur);

private:
	static std::shared_ptr<DescriptorHeap>								Heap;
	static std::shared_ptr<DescriptorHeap>								RTVHeap;
	static std::unique_ptr<MeshBuffer>									Screen;
	static std::unique_ptr<RootSignature>								RootSignatureData;
	static std::vector<std::unique_ptr<Pipeline>>						PipelineData;
	static std::vector<std::unique_ptr<RenderTarget>>					GaussRTVs;
	static BlurParam::GaussParam										Weights;
	static std::vector<std::unique_ptr<class ConstantBuffer>>			Params;

};

#endif