
// Scene
#include "SceneBase.h"
#include "SceneManager.h"

// Model
#include "RenderingComponent.h"

// System/GameObject
#include "GameObject.h"
// System/Rendering
#include "RenderingEngine.h"

void GameObject::InitBase()
{
	Engine = SceneManager::GetCurrentScene()->GetRenderingEngine();
	Init();
}

void GameObject::UninitBase()
{
	for (std::shared_ptr<GameObject> child : ChildGameObjects)
	{
		child->UninitBase();
		child = nullptr;
	}
	for (std::shared_ptr<Component> component : Components)
	{
		component->Uninit();
		component = nullptr;
	}
	Uninit();

	ChildGameObjects.clear();
	Components.clear();
}

void GameObject::UpdateBase()
{
	for (std::shared_ptr<GameObject> child : ChildGameObjects)
	{
		child->UpdateBase();
	}
	for (std::shared_ptr<Component> component : Components)
	{
		component->Update();
	}
	Update();
}

void GameObject::DrawBase(DirectX::XMFLOAT4X4 ParentMatrix)
{
	DirectX::XMMATRIX scale, rot, trans;
	scale	= DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);
	rot		= DirectX::XMMatrixRotationRollPitchYaw(Rotation.x,Rotation.y, Rotation.z);
	trans	= DirectX::XMMatrixTranslation(Position.x, Position.y,Position.z);
	DirectX::XMStoreFloat4x4(
		&fx4World,
		scale * rot * trans * DirectX::XMLoadFloat4x4(&ParentMatrix)
	);
	WorldPosition = { fx4World._41,fx4World._42,fx4World._43 };

	for (std::shared_ptr<GameObject> child : ChildGameObjects)
	{
		child->DrawBase(fx4World);
	}

	Draw();
	for (std::shared_ptr<Component> component : Components)
	{
		component->Draw();
	}
}

void GameObject::RenderingBase()
{
	Rendering();
	for (std::weak_ptr<RenderingComponent> comp : RenderingComponents)
	{
		comp.lock()->Rendering();
	}
}

DirectX::XMFLOAT3 GameObject::GetForwardVector()
{
	DirectX::XMFLOAT4X4 rot;
	DirectX::XMStoreFloat4x4(&rot,
		DirectX::XMMatrixRotationRollPitchYaw(
			Rotation.x, Rotation.y, Rotation.z));

	DirectX::XMFLOAT3 forward;
	forward.x = rot._31;
	forward.y = rot._32;
	forward.z = rot._33;

	return forward;
}

bool GameObject::Destroy()
{
	if (bDestroy)
	{
		UninitBase();
		return true;
	}
	return false;
}

void GameObject::Add2RenderingEngine(UINT timing, UINT passType)
{
	Engine->AddRenderObject(*this, timing, passType);
}

void GameObject::RegisterComponent2RenderingEngine(std::shared_ptr<class RenderingComponent> comp)
{
	Engine->RegisterRenderingComponentRef(comp);
}
