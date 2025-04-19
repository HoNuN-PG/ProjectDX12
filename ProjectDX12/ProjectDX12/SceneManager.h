#ifndef __SCENE_H__
#define __SCENE_H__

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
		LSYSTEMNEO,
		PROCEDURAL,
		FIELD,

		MAX_SCENES
	};
	int mCurrentScene = IBL;
	std::vector<std::unique_ptr<SceneBase>> mScenes;

public:
	SceneManager();

	HRESULT Init();
	void Uninit();
	void Update();
	void Draw();

};

#endif