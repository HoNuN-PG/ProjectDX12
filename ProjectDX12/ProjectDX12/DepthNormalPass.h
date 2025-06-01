#ifndef ___DEPTH_NORMAL_PASS_H___
#define ___DEPTH_NORMAL_PASS_H___

#include <vector>

#include "renderingPass.h"
#include "RenderingEngine.h"

class OpaqueDepthNormalPass : public RenderingPass
{
public:
	OpaqueDepthNormalPass();
	~OpaqueDepthNormalPass() {};
public:
	void Execute() override;
public:
	void AddObj(GameObject& obj) override;
private:
	std::vector<RenderingEngine::RenderingInfo> RenderObjects;

private:
	std::shared_ptr<RenderTarget> Depth;
	std::shared_ptr<RenderTarget> Normal;

};

#endif
