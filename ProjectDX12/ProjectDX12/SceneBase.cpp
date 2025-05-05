
#include "SceneBase.h"
#include "RenderingEngine.h"

HRESULT SceneBase::InitBase()
{
	Engine = std::make_unique<RenderingEngine>();
	Engine->Init();

	Init();

	return E_NOTIMPL;
}

void SceneBase::UninitBase()
{
	for (auto& objectList : GameObjects)
	{
		for (GameObject* object : objectList)
		{
			object->UninitBase();
			delete object;
		}
		objectList.clear();
	}
	Uninit();
	Engine->Uninit();
}

void SceneBase::UpdateBase()
{
	for (int i = 0; i < MAX_LAYER; i++)
	{
		for (GameObject* object : GameObjects[i])
		{
			object->UpdateBase();
		}
	}
	for (auto& objectList : GameObjects)
	{
		objectList.remove_if(
			[](GameObject* object) {return object->Destroy(); });
	}
	Update();
	Engine->Update();
}

void SceneBase::DrawBase()
{
	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixIdentity());
	for (int i = 0; i < MAX_LAYER; i++)
	{
		for (GameObject* object : GameObjects[i])
		{
			object->DrawBase(matrix);
		}
	}
	Draw();
	Engine->Draw();
}

RenderingEngine* SceneBase::GetRenderingEngine()
{
	return Engine.get();
}
