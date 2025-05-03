#ifndef ___SCENE_BACE_H___
#define ___SCENE_BACE_H___

#include <Windows.h>
#include <vector>
#include <memory>

#include "CameraBase.h"
#include "LightBase.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"

#include <vector>
#include <string>
#include <unordered_map>

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
	void WriteGlobalResource();

protected:
	static DescriptorHeap* GetGlobalHeap()
	{ return GlobalHeap.get(); }
	static ConstantBuffer* GetGlobalResource(UINT key);
private:
	static std::unique_ptr<DescriptorHeap>	GlobalHeap;
	static std::unordered_map<UINT,std::unique_ptr<ConstantBuffer>> GlobalResource;
protected:
	std::unique_ptr<CameraDebug> Camera;
	std::unique_ptr<LightBase> Light;

};

#endif