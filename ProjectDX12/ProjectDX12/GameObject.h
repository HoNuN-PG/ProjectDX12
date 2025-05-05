#ifndef ___GAMEOBJECT_H___
#define ___GAMEOBJECT_H___

#include <vector>
#include <list>
#include <memory>
#include <DirectXMath.h>

#include "Component.h"

class RenderingEngine;

class GameObject
{
public:
	enum RenderingTiming
	{
		CAM = 0,
		SKYBOX,
		LIGHT,
		DEFERRED,
		FORWARD,
		CANVAS,
		AFTER_POSTPROCESS,
		AFTER_FRAME_BUFFER,

		MAX_TIMING
	};

public:
	GameObject() {}
	virtual ~GameObject() {}

	void InitBase(RenderingTiming timing = RenderingTiming::FORWARD);
	void UninitBase();
	void UpdateBase();
	void DrawBase(DirectX::XMFLOAT4X4 ParentMatrix);
	void Rendering();

	virtual void Init() {}
	virtual void Uninit() {}
	virtual void Update() {}
	virtual void Draw() {}

protected:
	// トランスフォーム
	DirectX::XMFLOAT3 Position	= DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 Rotation	= DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 Scale		= DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT3 WorldPosition;						// ワールド座標
	DirectX::XMFLOAT4X4 fx4World;							// ワールドマトリクス

	// ゲームオブジェクト構成要素
	std::list<GameObject*> ChildGameObjects;				// 子オブジェクト
	std::list<Component*> Components;						// コンポーネント

	// 削除フラグ
	bool bDestroy = false;

public:
	// トランスフォーム
	DirectX::XMFLOAT3 GetPosition() { return Position; }
	DirectX::XMFLOAT3 GetRotation() { return Rotation; }
	DirectX::XMFLOAT3 GetScale() { return Scale; }
	void SetPosition(DirectX::XMFLOAT3 position) { Position = position; }
	void SetRotation(DirectX::XMFLOAT3 rotation) { Rotation = rotation; }
	void SetScale(DirectX::XMFLOAT3 scale) { Scale = scale; }
	DirectX::XMFLOAT3 GetWorldPosition() { return WorldPosition; }
	DirectX::XMFLOAT4X4 GetWorldMatrix() { return fx4World; }
	DirectX::XMFLOAT3 GetForwardVector();

	// 子オブジェクトの追加
	template <typename T>
	T* AddChild()
	{
		T* child = new T();
		ChildGameObjects.push_back(child);
		child->InitBase();
		return child;
	}

	// コンポーネントの追加・取得
	template <typename T>
	T* AddComponent()
	{
		T* component = new T(this);
		Components.push_back(component);
		((Component*)component)->Init();

		return component;
	}
	template <typename T>
	T* GetComponent()
	{
		for (Component* component : Components)
		{ // 探索
			if (typeid(*component) == typeid(T))
			{
				return (T*)component;
			}
		}
		return nullptr;
	}
	template <typename T>
	std::vector<T*> GetComponents()
	{
		std::vector<T*> components;
		for (Component* component : Components)
		{ // 探索
			if (typeid(*component) == typeid(T))
			{
				components.push_back(component);
			}
		}
		return components;
	}
	template <typename T>
	T* GetComponentDynamic()
	{
		for (Component* component : Components)
		{ // 探索
			if (dynamic_cast<T*>(component))
				return (T*)component;
		}
		return nullptr;
	}
	template <typename T>
	std::vector<T*> GetComponentsDynamic()
	{
		std::vector<T*> components;
		for (Component* component : Components)
		{ // 探索
			if (dynamic_cast<T*>(component))
			{
				components.push_back((T*)component);
			}
		}
		return components;
	}

	// 削除関数
	void SetDestroy() { bDestroy = true; }
	bool IsDestroy() { return bDestroy; }
	bool Destroy();

private:
	RenderingEngine* Engine;
	RenderingTiming Timing;

};

#endif
