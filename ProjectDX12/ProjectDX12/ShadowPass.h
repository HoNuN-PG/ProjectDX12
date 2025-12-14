#ifndef ___SHADOW_PASS_H___
#define ___SHADOW_PASS_H___

#include <memory>
#include <vector>

// Material//Materials
#include "M_Shadow.h"

// System/Rendering/Pass
#include "renderingPass.h"
// System/Rendering/Pipeline
#include "DescriptorHeap.h"
// System/Rendering/Texture
#include "DepthStencil.h"
#include "RenderTarget.h"
// System/Rendering
#include "RenderingEngine.h"

#define SHADOW_MAP_COUNT (3)

class CameraBase;

class ShadowPass : public RenderingPass
{
public:
	enum TextureType
	{
		Near = 0,
		Middle,
		Far,

		MAX
	};
	enum VSMTextureType
	{
		NearVSM = 3,
		MiddleVSM,
		FarVSM,

		MAX_VSM
	};

public:
	explicit ShadowPass();
	~ShadowPass() {};
	void Execute() override;

public:
	virtual void Init(
		std::shared_ptr<DescriptorHeap> rtvHeap,
		std::shared_ptr<DescriptorHeap> srvHeap,
		std::shared_ptr<DescriptorHeap> dsvHeap
	) override;
	void AddObj(GameObject& obj) override;

public:
	virtual std::shared_ptr<RenderTarget> GetTexture(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureRTV(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureSRV(UINT idx) override;

private:
	DirectX::XMFLOAT4X4 CalcCrop(
		float depth,
		int area,
		DirectX::XMFLOAT4X4 lvp
	);
	DirectX::XMFLOAT4X4 CalcTexelSnappedCrop(
		float depth,
		int area,
		float res,
		DirectX::XMFLOAT4X4 lv
	);

	// ガウスパラメータ
private:
	int GaussIdx[TextureType::MAX];

	// シャドウパラメータ
public:
	void SetCascadeAreas(UINT idx, float value) { CascadeAreas[idx] = value; };
public:
	DirectX::XMFLOAT2 ShadowMapsSize[TextureType::MAX];
	DirectX::XMFLOAT2 VSMShadowMapsSize[TextureType::MAX];
	DXGI_FORMAT ShadowMapsFormat;
private:
	std::vector<float> CascadeAreas;
	ShadowParam::ShadowMapsParam ShadowMapsParam;
	ShadowParam::ShadowReceieverParam ShadowReceiveParam;

private:
	std::vector<std::unique_ptr<DepthStencil>> DSVs;
	std::vector<std::shared_ptr<RenderTarget>> ShadowMaps;
	std::vector<std::shared_ptr<RenderTarget>> VSMShadowMaps;

private:
	std::vector<RenderingEngine::RenderingInfo> RenderObjects;
	std::shared_ptr<CameraBase> pCamera;

};

#endif
