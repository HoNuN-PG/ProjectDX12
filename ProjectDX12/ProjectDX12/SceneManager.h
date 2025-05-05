#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <Windows.h>
#include <vector>
#include <memory>

#include "SceneBase.h"

class SceneManager
{
private:
	enum SceneType 
	{
		SANDBOXDX12 = 0,

		MAX_SCENES
	};
	static int CurrentScene;
	static std::vector<std::unique_ptr<SceneBase>> Scenes;

public:
	SceneManager();

	static HRESULT Init();
	static void Uninit();
	static void Update();
	static void Draw();

public:
	static SceneBase* GetCurrentScene()
	{ return Scenes[CurrentScene].get(); }

};

#endif