
#include <algorithm>

#include "Scene/SceneManager.h"
#include "Scene/Scenes/SceneSandBoxDX12.h"
#include "System/Rendering/RenderingEngine.h"
#include "System/Input.h"
#include "System/DirectX.h"

int SceneManager::CurrentScene = SceneManager::SceneType::SANDBOXDX12;
std::vector<std::unique_ptr<SceneBase>> SceneManager::Scenes(MAX_SCENES);

HRESULT SceneManager::Init()
{
	if (!Scenes[CurrentScene])
	{
		switch (CurrentScene) 
		{
		case SANDBOXDX12:
			Scenes[CurrentScene] = std::make_unique<SceneSandBoxDX12>();
			break;
		default:break;
		}	
	}
	if (!Scenes[CurrentScene])
	{
		return E_FAIL;
	}
	return Scenes[CurrentScene]->InitBase();
}

void SceneManager::Uninit()
{
	if (Scenes[CurrentScene])
	{
		Scenes[CurrentScene]->UninitBase();
		Scenes[CurrentScene].reset(nullptr);
	}
}

void SceneManager::Update()
{
	int preScene = CurrentScene;
	if (Input::GetKeyTrigger(VK_RIGHT))
		CurrentScene = min(CurrentScene + 1, MAX_SCENES - 1);
	if (Input::GetKeyTrigger(VK_LEFT))
		CurrentScene = max(0, CurrentScene - 1);

	if (CurrentScene != preScene) 
	{
		int newScene = CurrentScene;
		CurrentScene = preScene;
		// GPU‚Ìˆ—Š®—¹‚ð‘Ò‚Á‚Ä‚©‚çƒV[ƒ“‚ð”jŠü‚·‚é
		WaitForGpu();
		Uninit();
		CurrentScene = newScene;
		Init();
	}

	if(Scenes[CurrentScene]) 
		Scenes[CurrentScene]->UpdateBase();
}

void SceneManager::Draw()
{
	if (Scenes[CurrentScene]) Scenes[CurrentScene]->DrawBase();
}

std::shared_ptr<CameraBase> SceneManager::GetCamera()
{
	RenderingEngine* engine = GetRenderingEngine().get();
	return engine->GetCamera();
}
