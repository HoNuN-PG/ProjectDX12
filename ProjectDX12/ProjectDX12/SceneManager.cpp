
#include "SceneManager.h"

#include <algorithm>
#include "Input.h"

#include "SceneSandBoxDX12.h"

int SceneManager::CurrentScene = SceneManager::SceneType::SANDBOXDX12;
std::vector<std::unique_ptr<SceneBase>> SceneManager::Scenes;

SceneManager::SceneManager()
{
	// ƒV[ƒ“”z—ñ‚Ì‰Šú‰»
	Scenes.resize(MAX_SCENES);
	for(int i = 0;i < Scenes.size();i++)
	{
		Scenes[i] = nullptr;
	}
}

HRESULT SceneManager::Init()
{
	HRESULT hr = E_FAIL;
	if (!Scenes[CurrentScene])
	{
		switch (CurrentScene) {
		default:break;
		case SANDBOXDX12:Scenes[CurrentScene] = std::make_unique<SceneSandBoxDX12>();	break;
		}	
	}
	return Scenes[CurrentScene]->InitBase();
}

void SceneManager::Uninit()
{
	if (Scenes[CurrentScene])
	{
		Scenes[CurrentScene]->UninitBase();
		Scenes[CurrentScene].release();
	}
}

void SceneManager::Update()
{
	int preScene = CurrentScene;
	if (Input::GetKeyTrigger(VK_RIGHT))
		CurrentScene = min(CurrentScene + 1, MAX_SCENES - 1);
	if (Input::GetKeyTrigger(VK_LEFT))
		CurrentScene = max(0, CurrentScene - 1);
	if (CurrentScene != preScene) {
		int newScene = CurrentScene;
		CurrentScene = preScene;
		Uninit();
		CurrentScene = newScene;
		Init();
	}

	if(Scenes[CurrentScene]) Scenes[CurrentScene]->UpdateBase();
}

void SceneManager::Draw()
{
	if (Scenes[CurrentScene]) Scenes[CurrentScene]->DrawBase();
}
