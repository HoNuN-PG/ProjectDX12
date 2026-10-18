#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <memory>
#include <vector>
#include <Windows.h>

#include "Scene/SceneBase.h"

class CameraBase;

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

	// 静的クラスのためインスタンス化しない
	SceneManager() = delete;

	static HRESULT Init();
	static void Uninit();
	static void Update();
	static void Draw();

public:

	static SceneBase* GetCurrentScene(){ return Scenes[CurrentScene].get(); }
	static class RenderingEngine* GetRenderingEngine(){ return Scenes[CurrentScene]->GetRenderingEngine(); }

public:

	static std::shared_ptr<CameraBase> GetCamera();

};

#endif