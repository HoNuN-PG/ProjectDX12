

#include "SceneSandBoxDX12.h"

#include "GlobalResourceKey.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "ConstantBuffer.h"
#include "ConstantWVP.h"
#include "plane.h"
#include "sphere.h"
#include "Model.h"

// マテリアル
#include "M_Shadow.h"
#include "M_DepthNormal.h"
#include "CustomDepthNormalPass.h"
#include "M_Deffered_Albedo_Normal.h"
#include "M_SimpleLit.h"
#include "M_Grid.h"
#include "M_SkyBox.h"

// ポストプロセス
#include "vignette.h"

// パス
#include "CustomDepthNormalPass.h"

HRESULT SceneSandBoxDX12::Init()
{
	// ボリューム追加
	GetRenderingEngine()->AddVolume<Vignette>();
	// パス追加
	GetRenderingEngine()->AddRenderingPass<CustomDepthNormalPass>(
		Material::RenderingTiming::AfterOpaqueDepthNormal,
		RenderingPass::RenderingPassType::CustomDepthNormal);

	// マテリアル作成
	// SkyBox
	std::shared_ptr<M_SkyBox> sky_box = std::make_shared<M_SkyBox>();
	Material::Initialize(sky_box, Heap.get(), Material::RenderingTiming::Environment);
	sky_box->AddTexture("assets/texture/HDRI/skybox2.hdr");

	// ShadowMap
	std::shared_ptr<M_SimpleShadowMaps> shadow_map = std::make_shared<M_SimpleShadowMaps>();
	Material::Initialize(shadow_map, Heap.get(), Material::RenderingTiming::Shadow);

	// DepthNormal
	std::shared_ptr<M_DepthNormal> opaque_depth_normal = std::make_shared<M_DepthNormal>();
	Material::Initialize(opaque_depth_normal, Heap.get(), Material::RenderingTiming::OpaqueDepthNormal);

	// CustomDepthNormal
	std::shared_ptr<M_DepthNormal> custom_opaque_depth_normal = std::make_shared<M_DepthNormal>();
	Material::Initialize(custom_opaque_depth_normal, Heap.get(),
		Material::RenderingTiming::AfterOpaqueDepthNormal,
		RenderingPass::RenderingPassType::CustomDepthNormal);

	// Grid
	std::shared_ptr<M_Grid> grid = std::make_shared<M_Grid>();
	Material::Initialize(grid, Heap.get());
	grid->SetGridSize(1);
	grid->SetSubGridSize(5);

	std::shared_ptr<M_GridShadow> grid_shadow = std::make_shared<M_GridShadow>();
	Material::Initialize(grid_shadow, Heap.get());
	grid_shadow->SetGridSize(1);
	grid_shadow->SetSubGridSize(5);

	std::shared_ptr<M_GridShadowVSM> grid_shadow_vsm = std::make_shared<M_GridShadowVSM>();
	Material::Initialize(grid_shadow_vsm, Heap.get());
	grid_shadow_vsm->SetGridSize(1);
	grid_shadow_vsm->SetSubGridSize(5);

	// SimpleLit
	std::shared_ptr<M_SimpleLit> simple_lit = std::make_shared<M_SimpleLit>();
	Material::Initialize(simple_lit, Heap.get());

	// モデル作成
	{// スカイボックス
		std::vector<std::shared_ptr<Material>> materials;
		materials.push_back(sky_box);

		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>(SceneBase::Layer::Environment);
		obj->SetPosition({ 0,0,0 });
		obj->SetRotation({ 0,0,0 });
		obj->SetScale({ 100,100,100 });
		std::shared_ptr<Sphere> model = GameObject::AddComponent<Sphere>(obj);
		model->Create(materials);
	}
	{// グリッド
		std::vector<std::shared_ptr<Material>> materials;
		materials.push_back(opaque_depth_normal);
		materials.push_back(grid_shadow_vsm);

		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,-1,0 });
		obj->SetRotation({ DirectX::XMConvertToRadians(90),0,0 });
		obj->SetScale({100,100,1});
		std::shared_ptr<Plane> model = GameObject::AddComponent<Plane>(obj);
		model->Create(materials);
	}
	{// 木
		std::vector<std::vector<std::shared_ptr<Material>>> meshmaterials;
		std::vector<std::shared_ptr<Material>> materials;
		materials.push_back(shadow_map);
		materials.push_back(opaque_depth_normal);
		materials.push_back(custom_opaque_depth_normal);
		materials.push_back(simple_lit);

		meshmaterials.push_back(materials);
		meshmaterials.push_back(materials);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,-2.5f,0 });
		obj->SetScale({2.5f,2.5f,2.5f});
		std::shared_ptr<Model> model = GameObject::AddComponent<Model>(obj);
		model->Create(meshmaterials, "assets/model/tree/height_tree.fbx");
	}
	{// 牛
		// Deffered
		std::shared_ptr<Material> deffered_albedo_normal;
		deffered_albedo_normal = std::make_shared<M_Deffered_Albedo_Normal>();
		Material::Initialize(deffered_albedo_normal,Heap.get(), Material::RenderingTiming::Deffered);
		deffered_albedo_normal->AddTexture("assets/model/spot/spot_texture.png");
		
		std::vector<std::vector<std::shared_ptr<Material>>> meshmaterials;
		std::vector<std::shared_ptr<Material>> materials;
		materials.push_back(opaque_depth_normal);
		materials.push_back(deffered_albedo_normal);
		
		meshmaterials.push_back(materials);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,0,10 });
		std::shared_ptr<Model> model = GameObject::AddComponent<Model>(obj);
		model->Create(meshmaterials, "assets/model/spot/spot.fbx");
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
