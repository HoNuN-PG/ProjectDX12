
#include "Scene/SceneBase.h"
#include "System/Rendering/RenderingEngine.h"

HRESULT SceneBase::InitBase()
{
	pEngine = std::make_unique<RenderingEngine>();
	pEngine->Init();

	// ディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = SCENE_HEAP_SIZE;
		pHeap = std::make_unique<DescriptorHeap>(desc);
	}

	return Init();
}

void SceneBase::UninitBase()
{
	Uninit();
	
	for (auto& objectList : GameObjects)
	{
		for (const std::shared_ptr<GameObject>& object : objectList)
		{
			object->UninitBase();
		}
		objectList.clear();
	}

	if (pEngine)
	{
		pEngine->Uninit();
		pEngine.reset();
	}
}

void SceneBase::UpdateBase()
{
	for (int i = 0; i < MAX_LAYER; i++)
	{
		for (const std::shared_ptr<GameObject>& object : GameObjects[i])
		{
			object->UpdateBase();
		}
	}
	for (auto& objectList : GameObjects)
	{
		objectList.remove_if(
			[](const std::shared_ptr<GameObject>& object) 
			{
				if (object->IsDestroy())
				{
					object->Destroy();
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
		for (const std::shared_ptr<GameObject>& object : GameObjects[i])
		{
			object->DrawBase(matrix);
		}
	}
	Draw();
	pEngine->Draw();
}

RenderingEngine* SceneBase::GetRenderingEngine()
{
	return pEngine.get();
}
