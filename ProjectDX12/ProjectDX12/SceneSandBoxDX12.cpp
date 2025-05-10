
#include "GlobalResourceKey.h"

#include "SceneSandBoxDX12.h"
#include "ConstantBuffer.h"
#include "DepthStencil.h"
#include "Pipeline.h"
#include "RootSignature.h"
#include "Model.h"

// マテリアル
#include "M_SimpleLit.h"
#include "M_Deffered_Albedo_Normal.h"

#include "ConstantWVP.h"

HRESULT SceneSandBoxDX12::Init()
{
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
		Materials.back()->Initialize(Heap.get(), Material::DEFERRED);
		Materials.back()->AddTexture("assets/model/spot/spot_texture.png");

		GameObject* obj = AddGameObject<GameObject>();
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
