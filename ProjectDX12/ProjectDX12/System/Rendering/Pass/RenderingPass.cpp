
#include "Scene/SceneManager.h"
#include "System/Rendering/Pass/RenderingPass.h"
#include "System/Rendering/RenderingEngine.h"

std::shared_ptr<RenderingEngine> RenderingPass::pEngine;

RenderingPass::RenderingPass()
{
	pEngine = SceneManager::GetCurrentScene()->GetRenderingEngine();
}
