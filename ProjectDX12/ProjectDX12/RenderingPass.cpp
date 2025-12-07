
// Scene
#include "SceneManager.h"

// System/Rendering/Pass
#include "RenderingPass.h"
// System/Rendering
#include "RenderingEngine.h"

std::shared_ptr<RenderingEngine> RenderingPass::Engine;

RenderingPass::RenderingPass()
{
	Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
}
