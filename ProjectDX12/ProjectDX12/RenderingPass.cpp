
#include "SceneManager.h"

#include "RenderingPass.h"

#include "RenderingEngine.h"

std::shared_ptr<RenderingEngine> RenderingPass::Engine;

RenderingPass::RenderingPass()
{
	Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
}
