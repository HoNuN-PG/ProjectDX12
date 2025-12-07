#ifndef ___GAMEOBJECT_H___
#define ___GAMEOBJECT_H___

#include <DirectXMath.h>
#include <list>
#include <memory>
#include <vector>
#include <Windows.h>

// System/Component
#include "Component.h"

class GameObject
{
public:
	GameObject() {}
	virtual ~GameObject() {}

	void InitBase();
	void UninitBase();
	void UpdateBase();
	void DrawBase(DirectX::XMFLOAT4X4 ParentMatrix);
	void BindRenderingEngine(UINT timing, UINT passType);
	void RenderingBase();
	void ReuseRenderingBase();

	virtual void Init() {}
	virtual void Uninit() {}
	virtual void Update() {}
	virtual void Draw() {}
	virtual void Rendering() {}
	virtual void ReuseRendering(){}

protected:
	// トランスフォーム
	DirectX::XMFLOAT3 Position	= DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 Rotation	= DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 Scale		= DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT3 WorldPosition;						// ワールド座標
	DirectX::XMFLOAT4X4 fx4World;							// ワールドマトリクス

	// ゲームオブジェクト構成要素
	std::list<std::shared_ptr<GameObject>> ChildGameObjects;				// 子オブジェクト
	std::list<std::shared_ptr<Component>> Components;						// コンポーネント

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
	std::shared_ptr<T> AddChild()
	{
		std::shared_ptr<T> child = std::make_shared<T>();
		ChildGameObjects.push_back(child);
		ChildGameObjects.back()->InitBase();
		return child;
	}

	// コンポーネントの追加・取得
	template <typename T>
	std::shared_ptr<T> AddComponent(std::shared_ptr<GameObject> own)
	{
		std::shared_ptr<T> component = std::make_shared<T>(own);
		own->Components.push_back(component);
		own->Components.back()->Init();

		// 描画コンポーネントの追加/登録
		if (std::shared_ptr<class RenderingComponent> c = std::dynamic_pointer_cast<RenderingComponent>(component))
		{
			RenderingComponents.push_back(c);
			AddRenderingComponent(c);
		}

		return component;
	}
	template <typename T>
	std::shared_ptr<T> GetComponent()
	{
		for (std::shared_ptr<Component> component : Components)
		{ // 探索
			if (std::shared_ptr<T> c = std::dynamic_pointer_cast<T>(component))
			{
				return c;
			}
		}
		return nullptr;
	}
	template <typename T>
	std::vector<std::shared_ptr<T>> GetComponents()
	{
		std::vector<std::shared_ptr<T>> components;
		for (std::shared_ptr<Component> component : Components)
		{ // 探索
			if (std::shared_ptr<T> c = std::dynamic_pointer_cast<T>(component))
			{
				components.push_back(c);
			}
		}
		return components;
	}

	// 削除関数
	void SetDestroy() { bDestroy = true; }
	bool IsDestroy() { return bDestroy; }
	bool Destroy();

private:
	void AddRenderingComponent(std::shared_ptr<class RenderingComponent> comp);

private:
	std::vector<std::weak_ptr<class RenderingComponent>> RenderingComponents;
	std::shared_ptr<class RenderingEngine> Engine;

};

#endif
