
#include "SceneManager.h"

#include "Input.h"
#include <algorithm>

#include "SceneIBL.h"
#include "SceneFractal.h"
#include "SceneLSystem.h"
#include "SceneLSystem3D.h"
#include "SceneProcedural.h"
#include "SceneField.h"
#include "SceneSandBoxDX12.h"

SceneManager::SceneManager()
{
	// ÉVÅ[ÉìîzóÒÇÃèâä˙âª
	mScenes.resize(MAX_SCENES);
	for(int i = 0;i < mScenes.size();i++)
	{
		mScenes[i] = nullptr;
	}
}

HRESULT SceneManager::Init()
{
	HRESULT hr = E_FAIL;
	if (!mScenes[mCurrentScene])
	{
		switch (mCurrentScene) {
		default:break;
		case IBL:			mScenes[mCurrentScene] = std::make_unique<SceneIBL>();			break;
		case FRACTAL :		mScenes[mCurrentScene] = std::make_unique<SceneFractal>();		break;
		case LSYSTEM:		mScenes[mCurrentScene] = std::make_unique<SceneLSystem>();		break;
		case LSYSTEM3D:		mScenes[mCurrentScene] = std::make_unique<SceneLSystem3D>();	break;
		case PROCEDURAL:	mScenes[mCurrentScene] = std::make_unique<SceneProcedural>();	break;
		case FIELD:			mScenes[mCurrentScene] = std::make_unique<SceneField>();		break;
		case SANDBOXDX12:	mScenes[mCurrentScene] = std::make_unique<SceneSandBoxDX12>();	break;
		}	
	}
	return mScenes[mCurrentScene]->Init();
}

void SceneManager::Uninit()
{
	if (mScenes[mCurrentScene])
	{
		mScenes[mCurrentScene]->Uninit();
		mScenes[mCurrentScene].release();
	}
}

void SceneManager::Update()
{
	int preScene = mCurrentScene;
	if (Input::GetKeyTrigger(VK_RIGHT))
		mCurrentScene = min(mCurrentScene + 1, MAX_SCENES - 1);
	if (Input::GetKeyTrigger(VK_LEFT))
		mCurrentScene = max(IBL, mCurrentScene - 1);
	if (mCurrentScene != preScene) {
		int newScene = mCurrentScene;
		mCurrentScene = preScene;
		Uninit();
		mCurrentScene = newScene;
		Init();
	}

	if(mScenes[mCurrentScene]) mScenes[mCurrentScene]->Update();
}

void SceneManager::Draw()
{
	if (mScenes[mCurrentScene]) mScenes[mCurrentScene]->Draw();
}
