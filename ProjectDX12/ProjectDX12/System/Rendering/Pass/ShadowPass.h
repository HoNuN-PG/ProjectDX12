#ifndef ___SHADOW_PASS_H___
#define ___SHADOW_PASS_H___

#include <memory>
#include <vector>

#include "Material/Materials/M_Shadow.h"
#include "System/Rendering/Pass/RenderingPass.h"
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Texture/DepthStencil.h"
#include "System/Rendering/Texture/RenderTarget.h"
#include "System/Rendering/RenderingEngine.h"

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
		NearVSM,
		MiddleVSM,
		FarVSM,

		MAX
	};

public:

	explicit ShadowPass();
	~ShadowPass() {};
	void Execute() override;

public:

	virtual void Init(
		std::shared_ptr<DescriptorHeap> rtvHeap,
		std::shared_ptr<DescriptorHeap> stagingHeap,
		std::shared_ptr<DescriptorHeap> srvHeap,
		std::shared_ptr<DescriptorHeap> dsvHeap
	) override;
	void AddObj(GameObject& obj) override;

public:

	virtual std::shared_ptr<RenderTarget> GetTextureStaging(UINT idx) override;
	virtual std::shared_ptr<RenderTarget> GetTexture(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureStagingRTV(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureRTV(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureStagingSRV(UINT idx) override;
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

public:

	void SetCascadeAreas(UINT idx, float value) { CascadeAreas[idx] = value; };

public:

	DirectX::XMFLOAT2 ShadowMapsSize[TextureType::NearVSM];
	DirectX::XMFLOAT2 VSMShadowMapsSize[TextureType::NearVSM];

private:

	std::vector<float> CascadeAreas;
	ShadowParam::ShadowMapsParam ShadowMapsParam;
	ShadowParam::ShadowReceieverParam ShadowReceiveParam;

private:

	// テクスチャ
	std::vector<std::unique_ptr<DepthStencil>> DSVs;
	std::vector<std::shared_ptr<RenderTarget>> StagingShadowMaps;
	std::vector<std::shared_ptr<RenderTarget>> ShadowMaps;
	std::vector<std::shared_ptr<RenderTarget>> StagingVSMShadowMaps;
	std::vector<std::shared_ptr<RenderTarget>> VSMShadowMaps;

	// ガウスパラメータ
	int GaussIdx[TextureType::NearVSM];

private:

	std::shared_ptr<CameraBase> pCamera;

private:

	std::vector<RenderingEngine::RenderingInfo> RenderObjects;

};

#endif
