
#include "GlobalResourceKey.h"

#include "SceneSandBoxDX12.h"

#include "ConstantBuffer.h"
#include "DepthStencil.h"
#include "Pipeline.h"
#include "RootSignature.h"
#include "Model.h"

// マテリアル
#include "M_DepthNormal.h"
#include "M_Deffered_Albedo_Normal.h"
#include "M_SimpleLit.h"

#include "ConstantWVP.h"

#include "vignette.h"

HRESULT SceneSandBoxDX12::Init()
{
	// ボリューム追加
	GetRenderingEngine()->AddVolume<Vignette>();

	// モデル作成
	{
		std::vector<Material*> materials;
		Materials.push_back(std::make_unique<M_DepthNormal>());
		Materials.back()->Initialize(Heap.get(), RenderingPass::O_DEPTH_NORMAL_PASS);
		materials.push_back(Materials.back().get());
		Materials.push_back(std::make_unique<M_SimpleLit>());
		Materials.back()->Initialize(Heap.get());
		materials.push_back(Materials.back().get());

		GameObject* obj = AddGameObject<GameObject>();
		obj->SetPosition({ -1,0,0 });
		Model* model = obj->AddComponent<Model>();
		model->Create(materials, "assets/model/spot/spot.fbx");
	}
	{
		std::vector<Material*> materials;
		Materials.push_back(std::make_unique<M_DepthNormal>());
		Materials.back()->Initialize(Heap.get(), RenderingPass::O_DEPTH_NORMAL_PASS);
		materials.push_back(Materials.back().get());
		Materials.push_back(std::make_unique<M_Deffered_Albedo_Normal>());
		Materials.back()->Initialize(Heap.get(), RenderingPass::MAIN, Material::DEFERRED);
		Materials.back()->AddTexture("assets/model/spot/spot_texture.png");
		materials.push_back(Materials.back().get());

		GameObject* obj = AddGameObject<GameObject>();
		obj->SetPosition({ 1,0,0 });
		Model* model = obj->AddComponent<Model>();
		model->Create(materials, "assets/model/spot/spot.fbx");
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
