
// Scene
#include "SceneBase.h"

// System/Rendering
#include "RenderingEngine.h"

HRESULT SceneBase::InitBase()
{
	pEngine = std::make_shared<RenderingEngine>();
	pEngine->Init();

	// ディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 256;
		pHeap = std::make_shared<DescriptorHeap>(desc);
	}

	Init();
	return E_NOTIMPL;
}

void SceneBase::UninitBase()
{
	for (auto& objectList : GameObjects)
	{
		for (std::shared_ptr<GameObject> object : objectList)
		{
			object->UninitBase();
			object = nullptr;
		}
		objectList.clear();
	}
	Uninit();
	pEngine->Uninit();
}

void SceneBase::UpdateBase()
{
	for (int i = 0; i < MAX_LAYER; i++)
	{
		for (std::shared_ptr<GameObject> object : GameObjects[i])
		{
			object->UpdateBase();
		}
	}
	for (auto& objectList : GameObjects)
	{
		objectList.remove_if(
			[](std::shared_ptr<GameObject> object) 
			{
				if (object->IsDestroy())
				{
					object->Destroy();
					object = nullptr;
					return true;
				}
				return false;
			}
		);
	}
	Update();
	pEngine->Update();
}

void SceneBase::DrawBase()
{
	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixIdentity());
	for (int i = 0; i < MAX_LAYER; i++)
	{
		for (std::shared_ptr<GameObject> object : GameObjects[i])
		{
			object->DrawBase(matrix);
		}
	}
	Draw();
	pEngine->Draw();
}

std::shared_ptr<RenderingEngine> SceneBase::GetRenderingEngine()
{
	return pEngine;
}
