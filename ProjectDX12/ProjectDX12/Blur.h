#ifndef ___BLUR_H___
#define ___BLUR_H___

#include <memory>
#include <vector>

#include "RootSignature.h"
#include "Pipeline.h"
#include "MeshBuffer.h"
#include "RenderTarget.h"
#include "ConstantBuffer.h"

class DescriptorHeap;

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

	// ÉKÉEÉXÇé¿çsÇ≈Ç´ÇÈç≈ëÂêî
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
	static void Create();
	static void Destroy();
private:
	static std::unique_ptr<Gauss> Instance;

public:
	Gauss() {};
	~Gauss() {};
	static void ExecuteScreenGauss2(std::shared_ptr<RenderTarget> src, std::shared_ptr<RenderTarget> dest);
	static void CalcWeights(std::weak_ptr<float[]> weights, float blur);
	static void Refresh();

private:
	UINT														GaussIdx;
	std::shared_ptr<DescriptorHeap>								Heap;
	std::shared_ptr<DescriptorHeap>								RTVHeap;
	std::unique_ptr<MeshBuffer>									Screen;
	std::unique_ptr<RootSignature>								RootSignatureData;
	std::vector<std::unique_ptr<Pipeline>>						PipelineData;
	std::vector<std::unique_ptr<RenderTarget>>					GaussRTVs;
	std::vector<std::unique_ptr<class ConstantBuffer>>			Params;
	BlurParam::GaussParam										Weights;

};

#endif