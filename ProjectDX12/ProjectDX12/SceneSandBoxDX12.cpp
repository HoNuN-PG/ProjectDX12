
#include "GlobalResourceKey.h"

#include "SceneSandBoxDX12.h"
#include "ConstantWVP.h"

// マテリアル
#include "M_SimpleLit.h"
#include "M_Deffered_Albedo_Normal.h"

HRESULT SceneSandBoxDX12::Init()
{
	// ディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 128;
		Heap = std::make_unique<DescriptorHeap>(desc);
	}

	// モデル作成
	{
		Materials.push_back(std::make_unique<M_SimpleLit>());
		Materials.back()->Initialize(Heap.get());

		GameObject* obj = AddGameObject<GameObject>();
		obj->SetPosition({ -1,0,0 });
		Model* model = obj->AddComponent<Model>();
		model->Create(Materials.back().get(), "assets/model/spot/spot.fbx");
	}
	{
		Materials.push_back(std::make_unique<M_Deffered_Albedo_Normal>());
		Materials.back()->Initialize(Heap.get());
		Materials.back()->AddTexture("assets/model/spot/spot_texture.png");

		GameObject* obj = AddGameObject<GameObject>(GameObject::DEFERRED);
		obj->SetPosition({ 1,0,0 });
		Model* model = obj->AddComponent<Model>();
		model->Create(Materials.back().get(), "assets/model/spot/spot.fbx");
	}

    return E_NOTIMPL;
}

void SceneSandBoxDX12::Uninit()
{
}

void SceneSandBoxDX12::Update()
{
}

void SceneSandBoxDX12::Draw()
{
}
