#ifndef ___SCENE_BACE_H___
#define ___SCENE_BACE_H___

#include <Windows.h>
#include <memory>

#include "CameraBase.h"
#include "LightBase.h"

class SceneBase
{
public:
	virtual HRESULT Init()	= 0;
	virtual void Uninit()	= 0;
	virtual void Update()	= 0;
	virtual void Draw()		= 0;

protected:
	void SetUp();
protected:
	std::unique_ptr<CameraDebug> Camera;
	std::unique_ptr<LightBase> Light;

};

#endif