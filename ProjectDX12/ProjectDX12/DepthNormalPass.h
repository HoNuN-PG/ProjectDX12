#ifndef ___DEPTH_NORMAL_PASS_H___
#define ___DEPTH_NORMAL_PASS_H___

#include <vector>

// System/Rendering/Pass
#include "renderingPass.h"
// System/Rendering
#include "RenderingEngine.h"

class OpaqueDepthNormalPass : public RenderingPass
{
public:

	enum TextureType
	{
		DepthTexture = 0,
		NormalTexture,

		MAX
	};

public:

	OpaqueDepthNormalPass();
	~OpaqueDepthNormalPass() {};
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

	std::shared_ptr<DepthStencil> DSV;
	std::shared_ptr<RenderTarget> Depth;
	std::shared_ptr<RenderTarget> Normal;

private:

	std::vector<RenderingEngine::RenderingInfo> RenderObjects;

};

#endif
