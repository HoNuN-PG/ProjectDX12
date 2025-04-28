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
		IBL = 0,
		FRACTAL,
		LSYSTEM,
		LSYSTEM3D,
		PROCEDURAL,
		FIELD,
		SANDBOXDX12,

		MAX_SCENES
	};
	int mCurrentScene = SANDBOXDX12;
	std::vector<std::unique_ptr<SceneBase>> mScenes;

public:
	SceneManager();

	HRESULT Init();
	void Uninit();
	void Update();
	void Draw();

};

#endif