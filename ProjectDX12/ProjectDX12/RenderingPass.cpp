
// Scene
#include "SceneManager.h"

// System/Rendering/Pass
#include "RenderingPass.h"
// System/Rendering
#include "RenderingEngine.h"

std::shared_ptr<RenderingEngine> RenderingPass::pEngine;

RenderingPass::RenderingPass()
{
	pEngine = SceneManager::GetCurrentScene()->GetRenderingEngine();
}
