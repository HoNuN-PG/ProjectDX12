#ifndef ___SCENE_BACE_H___
#define ___SCENE_BACE_H___

#include <Windows.h>
#include <vector>
#include <list>
#include <array>
#include <memory>
#include <unordered_map>

#include "GameObject.h"

#include "RenderingEngine.h"

class SceneBase
{
public:
	enum Layer
	{
		Camera = 0,
		Environment,
		Opaque,
		OpaqueInstance,
		Translucent,
		TranslucentInstance,

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

	// ゲームオブジェクト
public:
	template <typename T>
	std::shared_ptr<T> AddGameObject(Layer layer = Opaque)
	{
		std::shared_ptr<T> gameObject = std::make_shared<T>();
		GameObjects[layer].push_back(gameObject);
		GameObjects[layer].back()->InitBase();
		return gameObject;
	}
	template <typename T>
	std::shared_ptr<T> GetGameObject()
	{
		for (auto& objectList : GameObjects)
		{
			for (std::shared_ptr<GameObject> object : objectList)
			{
				if (std::shared_ptr<T> o = std::dynamic_pointer_cast<T>(object))
				{
					return o;
				}
			}
		}
		return nullptr;
	}
	template <typename T>
	std::vector<std::shared_ptr<T>> GetGameObjects()
	{
		std::vector<std::shared_ptr<T>> objects;
		for (auto& objectList : GameObjects)
		{
			for (std::shared_ptr<GameObject> object : objectList)
			{
				if (std::shared_ptr<T> o = std::dynamic_pointer_cast<T>(object))
				{
					objects.push_back(o);
				}
			}
		}
		return objects;
	}
protected:
	std::array<std::list<std::shared_ptr<GameObject>>, MAX_LAYER> GameObjects;

	// レンダリングエンジン
public:
	std::shared_ptr<RenderingEngine> GetRenderingEngine();
private:
	std::shared_ptr<RenderingEngine> Engine;

	// シーンヒープ
public:
	std::shared_ptr<DescriptorHeap> GetHeap()
	{ return Heap; }
protected:
	std::shared_ptr<DescriptorHeap>	Heap;

};

#endif