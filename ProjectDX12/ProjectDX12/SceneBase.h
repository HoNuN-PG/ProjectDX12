#ifndef ___SCENE_BACE_H___
#define ___SCENE_BACE_H___

#include <Windows.h>
#include <vector>
#include <memory>

#include "CameraBase.h"
#include "LightBase.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"

class SceneBase
{
public:
	SceneBase(){}
	virtual ~SceneBase() {};
	virtual HRESULT Init()	= 0;
	virtual void Uninit()	= 0;
	virtual void Update()	= 0;
	virtual void Draw()		= 0;

protected:
	void Initialize();
	void SetUpResource();
protected:
	std::unique_ptr<CameraDebug> Camera;
	std::unique_ptr<LightBase> Light;
protected:
	DescriptorHeap* GetGlobalHeap()
	{ return GlobalHeap.get(); }
	ConstantBuffer* GetGlobalResource(int _resourceID)
	{ return GlobalResource[_resourceID].get(); }
private:
	std::unique_ptr<DescriptorHeap>	GlobalHeap;
	std::vector<std::unique_ptr<ConstantBuffer>> GlobalResource;

};

#endif