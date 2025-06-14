
#include "DepthNormalPass.h"

#include "GlobalResourceKey.h"

#include "GameObject.h"

OpaqueDepthNormalPass::OpaqueDepthNormalPass()
{
	PassType = RenderingPass::RenderingPassType::OpaqueDepthNormal;
	Depth = Engine->GetGlobalRenderTarget(GlobalTextureResourceKey::DepthTexture);
	Normal = Engine->GetGlobalRenderTarget(GlobalTextureResourceKey::NormalTexture);
}

void OpaqueDepthNormalPass::Execute()
{
	// DepthNormal
	for (int i = 0; i < RenderObjects.size(); ++i)
	{
		RenderObjects[i].obj.RenderingBase();
	}
	RenderObjects.clear();
}

void OpaqueDepthNormalPass::AddObj(GameObject& obj)
{
	RenderingEngine::RenderingInfo info = { obj };
	RenderObjects.push_back(info);
}
