#ifndef ___GAMEOBJECT_H___
#define ___GAMEOBJECT_H___

#include <DirectXMath.h>
#include <list>
#include <memory>
#include <vector>
#include <Windows.h>

#include "System/Component/Component.h"

class GameObject
{
public:

	GameObject() {}
	virtual ~GameObject() {}

	void InitBase(class RenderingEngine* engine);
	void UninitBase();
	void UpdateBase();
	void DrawBase(DirectX::XMFLOAT4X4 ParentMatrix);
	void RenderingBase();

	virtual void Init() {}
	virtual void Uninit() {}
	virtual void Update() {}
	virtual void Draw() {}
	virtual void Rendering() {}

	// =================================================
	// トランスフォーム
public:

	DirectX::XMFLOAT3 GetPosition() { return Position; }
	DirectX::XMFLOAT3 GetRotation() { return Rotation; }
	DirectX::XMFLOAT3 GetScale() { return Scale; }
	void SetPosition(DirectX::XMFLOAT3 position) { Position = position; }
	void SetRotation(DirectX::XMFLOAT3 rotation) { Rotation = rotation; }
	void SetScale(DirectX::XMFLOAT3 scale) { Scale = scale; }
	DirectX::XMFLOAT3 GetWorldPosition() { return WorldPosition; }
	DirectX::XMFLOAT4X4 GetWorldMatrix() { return fx4World; }
	DirectX::XMFLOAT3 GetForwardVector();

protected:

	DirectX::XMFLOAT3 Position	= DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 Rotation	= DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 Scale		= DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT3 WorldPosition; // ワールド座標
	DirectX::XMFLOAT4X4 fx4World;	 // ワールドマトリクス

	// =================================================
	// 削除
public:

	void SetDestroy() { bDestroy = true; }
	bool IsDestroy() { return bDestroy; }
	bool Destroy();

protected:

	bool bDestroy = false;

	// =================================================
	// ゲームオブジェクト構成要素
protected:

	std::list<std::shared_ptr<GameObject>> ChildGameObjects;				// 子オブジェクト
	std::list<std::shared_ptr<Component>> Components;						// コンポーネント

public:

	// 子オブジェクトの追加
	template <typename T>
	std::shared_ptr<T> AddChild()
	{
		std::shared_ptr<T> child = std::make_shared<T>();
		ChildGameObjects.push_back(child);
		ChildGameObjects.back()->InitBase(pEngine);
		return child;
	}

public:

	// コンポーネントの追加・取得
	template <typename T>
	std::shared_ptr<T> AddComponent()
	{
		std::shared_ptr<T> component = std::make_shared<T>(this);
		this->Components.push_back(component);
		this->Components.back()->Init();

		// 描画コンポーネントの追加/登録
		if (RenderingComponent* c = dynamic_cast<RenderingComponent*>(component.get()))
		{
			RenderingComponents.push_back(c);
		}

		return component;
	}
	template <typename T>
	std::shared_ptr<T> GetComponent()
	{
		for (const std::shared_ptr<Component>& component : Components)
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
		for (const std::shared_ptr<Component>& component : Components)
		{ // 探索
			if (std::shared_ptr<T> c = std::dynamic_pointer_cast<T>(component))
			{
				components.push_back(c);
			}
		}
		return components;
	}

	// =================================================
	// レンダリングエンジン
public:

	// レンダリングエンジンにオブジェクトを描画登録
	void Add2RenderingEngine(UINT timing, UINT passType);

private:

	class RenderingEngine* pEngine = nullptr; // レンダリングエンジンとゲームオブジェクトはシーン管理でありレンダリングエンジンの方が寿命が長いため参照が保証されている
	std::vector<class RenderingComponent*> RenderingComponents;	// 描画コンポーネント（Componentsが所有するため参照が保証されている）

};

#endif
