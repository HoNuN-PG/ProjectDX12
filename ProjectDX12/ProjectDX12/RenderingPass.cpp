
#include "RenderingPass.h"

#include "SceneManager.h"
#include "RenderingEngine.h"

std::shared_ptr<RenderingEngine> RenderingPass::Engine;

RenderingPass::RenderingPass()
{
	Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
}
