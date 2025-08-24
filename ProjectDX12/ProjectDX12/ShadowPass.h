#ifndef ___SHADOW_PASS_H___
#define ___SHADOW_PASS_H___

#include "renderingPass.h"

#include <vector>
#include <memory>

#include "RenderingEngine.h"
#include "DescriptorHeap.h"
#include "DepthStencil.h"
#include "RenderTarget.h"

#include "M_Shadow.h"

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
public:
	void Execute() override;
private:
	DirectX::XMFLOAT4X4 CalcCrop(
		float depth,
		int area,
		DirectX::XMFLOAT4X4 lvp);
public:
	virtual void Init(
		std::shared_ptr<DescriptorHeap> rtvHeap,
		std::shared_ptr<DescriptorHeap> srvHeap,
		std::shared_ptr<DescriptorHeap> dsvHeap) override;
	void AddObj(GameObject& obj) override;
	void SetCascadeAreas(UINT idx,float value)
	{CascadeAreas[idx] = value;};
	virtual std::shared_ptr<RenderTarget> GetTexture(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureRTV(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureSRV(UINT idx) override;
private:
	std::vector<RenderingEngine::RenderingInfo> RenderObjects;

private:
	std::vector<float> CascadeAreas;
	std::shared_ptr<CameraBase> pCamera;
	std::vector<std::unique_ptr<DepthStencil>> DSVs;
	std::vector<std::shared_ptr<RenderTarget>> ShadowMaps;
	std::vector<std::shared_ptr<RenderTarget>> VSMShadowMaps;

private:
	ShadowParam::ShadowMapsParam ShadowMapsParam;
	ShadowParam::ShadowReceieverParam ShadowReceiveParam;

public:
	static DirectX::XMFLOAT2 ShadowMapsSize[TextureType::Far + 1];
	static DXGI_FORMAT ShadowMapsFormat;

};

#endif
