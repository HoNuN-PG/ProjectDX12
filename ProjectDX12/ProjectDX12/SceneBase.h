#ifndef ___SCENE_BACE_H___
#define ___SCENE_BACE_H___

#include <Windows.h>
#include <vector>
#include <memory>
#include <array>
#include <list>

#include "GameObject.h"
#include "CameraBase.h"
#include "LightBase.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "DepthStencil.h"

#include <string>
#include <unordered_map>

class SceneBase
{
public:
	enum Layer
	{
		CAM = 0,
		SKYBOX,
		LIGHT,
		OPACITY,
		OPACITY_INC,
		TRANSPARENCY,
		TRANSPARENCY_INC,
		DEPTH,
		UI,

		MAX_LAYER,
	};

public:
	SceneBase(){}
	virtual ~SceneBase() {};

	HRESULT InitBase();
	void UninitBase();
	void UpdateBase();
	void DrawBase();

	virtual HRESULT Init()	= 0;
	virtual void Uninit()	= 0;
	virtual void Update()	= 0;
	virtual void Draw()		= 0;

protected:
	void WriteGlobalResource();

public:
	template <typename T>
	T* AddGameObject(Layer layer = OPACITY)
	{
		T* gameObject = new T();
		GameObjects[layer].push_back(gameObject);
		gameObject->InitBase();
		return gameObject;
	}
	template <typename T>
	T* GetGameObject()
	{
		for (auto& objectList : GameObjects)
		{
			for (GameObject* object : objectList)
			{
				if (typeid(*object) == typeid(T))
				{
					return (T*)object;
				}
			}
		}
		return nullptr;
	}
	template <typename T>
	std::vector<T*> GetGameObjects()
	{
		std::vector<T*> objects;
		for (auto& objectList : GameObjects)
		{
			for (GameObject* object : objectList)
			{
				if (typeid(*object) == typeid(T))
				{
					objects.push_back((T*)object);
				}
			}
		}
		return objects;
	}
protected:
	std::array<std::list<GameObject*>, MAX_LAYER> GameObjects;

protected:
	std::unique_ptr<DescriptorHeap>					DSVHeap;
	std::unique_ptr<DepthStencil>					DSV;

public:
	static DescriptorHeap* GetGlobalHeap()
	{ return GlobalHeap.get(); }
	static ConstantBuffer* GetGlobalResource(UINT key);
private:
	static std::unique_ptr<DescriptorHeap>	GlobalHeap;
	static std::unordered_map<UINT,std::unique_ptr<ConstantBuffer>> GlobalResource;
protected:
	CameraDebug* Camera;
	LightBase* Light;

};

#endif