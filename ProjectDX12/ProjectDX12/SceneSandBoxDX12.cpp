
#include "GlobalResourceKey.h"

#include "SceneSandBoxDX12.h"
#include "ConstantWVP.h"

// マテリアル
#include "Material/M_SimpleLit.h"

HRESULT SceneSandBoxDX12::Init()
{
	// ディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 128;
		Heap = std::make_unique<DescriptorHeap>(desc);
	}

	// マテリアル作成
	Materials.push_back(std::make_unique<M_SimpleLit>());
	Materials.back()->Initialize(Heap.get());

	// モデル作成
	GameObject* obj = AddGameObject<GameObject>();
	obj->SetPosition({ 0,0,0 });
	Model* model = obj->AddComponent<Model>();
	model->Create(Materials[0].get(), "assets/model/spot/spot.fbx");

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
