
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
		std::shared_ptr<Material> opaque_depth_normal;
		opaque_depth_normal = std::make_shared<M_DepthNormal>();
		opaque_depth_normal->Initialize(Heap.get(), Material::RenderingTiming::OpaqueDepthNormal);

		std::shared_ptr<Material> simple_lit;
		simple_lit = std::make_shared<M_SimpleLit>();
		simple_lit->Initialize(Heap.get());

		std::vector<std::shared_ptr<Material>> materials;
		materials.push_back(opaque_depth_normal);
		materials.push_back(simple_lit);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ -1,0,0 });
		std::shared_ptr<Model> model = obj->AddComponent<Model>();
		model->Create(materials, "assets/model/spot/spot.fbx");
	}
	{
		std::shared_ptr<Material> opaque_depth_normal;
		opaque_depth_normal = std::make_shared<M_DepthNormal>();
		opaque_depth_normal->Initialize(Heap.get(), Material::RenderingTiming::OpaqueDepthNormal);

		std::shared_ptr<Material> deffered_albedo_normal;
		deffered_albedo_normal = std::make_shared<M_Deffered_Albedo_Normal>();
		deffered_albedo_normal->Initialize(Heap.get(), Material::RenderingTiming::Deffered);
		deffered_albedo_normal->AddTexture("assets/model/spot/spot_texture.png");

		std::vector<std::shared_ptr<Material>> materials;
		materials.push_back(opaque_depth_normal);
		materials.push_back(deffered_albedo_normal);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 1,0,0 });
		std::shared_ptr<Model> model = obj->AddComponent<Model>();
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
