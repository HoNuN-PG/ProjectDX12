#ifndef ___CUSTOM_DEPTH_PASS_H___
#define ___CUSTOM_DEPTH_PASS_H___

#include <memory>
#include <vector>

#include "renderingPass.h"

#include "DescriptorHeap.h"

#include "DepthStencil.h"
#include "RenderTarget.h"

#include "RenderingEngine.h"

class CustomDepthNormalPass : public RenderingPass
{
public:
	enum TextureType
	{
		DepthTexture = 0,
		NormalTexture,

		MAX
	};
public:
	explicit CustomDepthNormalPass();
	~CustomDepthNormalPass() {};
public:
	void Execute() override;
public:
	virtual void Init(
		std::shared_ptr<DescriptorHeap> rtvHeap,
		std::shared_ptr<DescriptorHeap> srvHeap,
		std::shared_ptr<DescriptorHeap> dsvHeap) override;
	void AddObj(GameObject& obj) override;
	virtual std::shared_ptr<RenderTarget> GetTexture(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureRTV(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureSRV(UINT idx) override;
private:
	std::vector<RenderingEngine::RenderingInfo> RenderObjects;

private:
	std::unique_ptr<DepthStencil> DSV;
	std::shared_ptr<RenderTarget> Depth;
	std::shared_ptr<RenderTarget> Normal;

};

#endif
