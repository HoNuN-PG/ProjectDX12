#ifndef ___CUSTOM_DEPTH_PASS_H___
#define ___CUSTOM_DEPTH_PASS_H___

#include "renderingPass.h"

#include <vector>
#include <memory>

#include "RenderingEngine.h"

#include "DepthStencil.h"
#include "RenderTarget.h"

#include "DescriptorHeap.h"

class CustomDepthPass : public RenderingPass
{
public:
	enum TextureType
	{
		CustomDepth,

		MAX
	};
public:
	explicit CustomDepthPass(
		std::shared_ptr<DescriptorHeap> rtvHeap,
		std::shared_ptr<DescriptorHeap> srvHeap,
		std::shared_ptr<DescriptorHeap> dsvHeap);
	~CustomDepthPass() {};
public:
	void Execute() override;
public:
	void AddObj(GameObject& obj) override;
	virtual std::shared_ptr<RenderTarget> GetTexture(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureRTV(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureSRV(UINT idx) override;
private:
	std::vector<RenderingEngine::RenderingInfo> RenderObjects;

private:
	std::unique_ptr<DepthStencil> DSV;
	std::shared_ptr<RenderTarget> CustomDepthTexture;

};

#endif
