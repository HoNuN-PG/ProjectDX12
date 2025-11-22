
#include "FollowActorComponent.h"

#include "CameraBase.h"

// マテリアル
#include "M_Shadow.h"
#include "M_DepthNormal.h"
#include "CustomDepthNormalPass.h"
#include "M_Deffered_Albedo_Normal.h"
#include "M_SimpleLit.h"
#include "M_Grid.h"
#include "M_SkyBox.h"

#include "Model.h"
#include "plane.h"
#include "sphere.h"

#include "SceneSandBoxDX12.h"

#include "GameObject.h"

#include "ConstantBuffer.h"
#include "ConstantWVP.h"

#include "Pipeline.h"
#include "RootSignature.h"

// パス
#include "CustomDepthNormalPass.h"

// ポストプロセス
#include "vignette.h"

#include "GlobalResourceKey.h"

HRESULT SceneSandBoxDX12::Init()
{
	// ボリューム追加
	GetRenderingEngine()->AddVolume<Vignette>();
	// パス追加
	GetRenderingEngine()->AddRenderingPass<CustomDepthNormalPass>(
		Material::RenderingTiming::AfterOpaqueDepthNormal,
		RenderingPass::RenderingPassType::CustomDepthNormal);

	Material::Description desc;

	// マテリアル作成
	// SkyBox
	desc.cull = D3D12_CULL_MODE_FRONT;
	std::shared_ptr<M_SkyBox> sky_box = std::make_shared<M_SkyBox>();
	Material::Initialize(sky_box, Heap.get(),desc, Material::RenderingTiming::Environment);
	sky_box->AddTexture("../exe/assets/texture/HDRI/skybox2.hdr");
	desc.cull = D3D12_CULL_MODE_BACK;

	// ShadowMap
	std::shared_ptr<M_SimpleShadowMaps> shadow_map = std::make_shared<M_SimpleShadowMaps>();
	Material::Initialize(shadow_map, Heap.get(), desc, Material::RenderingTiming::Shadow);
	// 葉
	desc.cull = D3D12_CULL_MODE_NONE;
	desc.WriteDepth = FALSE;
	std::shared_ptr<M_OpaqueSimpleShadowMaps> leaf_shadow_map = std::make_shared<M_OpaqueSimpleShadowMaps>();
	Material::Initialize(leaf_shadow_map, Heap.get(), desc, Material::RenderingTiming::Shadow);
	leaf_shadow_map->AddTexture("../exe/assets/model/tree/T_Leaves_Round_01_C.dds");
	desc.WriteDepth = TRUE;
	desc.cull = D3D12_CULL_MODE_BACK;

	// DepthNormal
	std::shared_ptr<M_DepthNormal> opaque_depth_normal = std::make_shared<M_DepthNormal>();
	Material::Initialize(opaque_depth_normal, Heap.get(), desc, Material::RenderingTiming::OpaqueDepthNormal);

	// CustomDepthNormal
	std::shared_ptr<M_DepthNormal> custom_opaque_depth_normal = std::make_shared<M_DepthNormal>();
	Material::Initialize(custom_opaque_depth_normal, Heap.get(), 
		desc,
		Material::RenderingTiming::AfterOpaqueDepthNormal,
		RenderingPass::RenderingPassType::CustomDepthNormal);

	// Grid
	std::shared_ptr<M_Grid> grid = std::make_shared<M_Grid>();
	Material::Initialize(grid, Heap.get(), desc);
	grid->SetGridSize(1);
	grid->SetSubGridSize(5);

	std::shared_ptr<M_GridShadow> grid_shadow = std::make_shared<M_GridShadow>();
	Material::Initialize(grid_shadow, Heap.get(), desc);
	grid_shadow->SetGridSize(1);
	grid_shadow->SetSubGridSize(5);

	std::shared_ptr<M_GridShadowVSM> grid_shadow_vsm = std::make_shared<M_GridShadowVSM>();
	Material::Initialize(grid_shadow_vsm, Heap.get(), desc);
	grid_shadow_vsm->SetGridSize(1);
	grid_shadow_vsm->SetSubGridSize(5);

	// SimpleLit
	std::shared_ptr<M_SimpleLit> simple_lit = std::make_shared<M_SimpleLit>();
	Material::Initialize(simple_lit, Heap.get(), desc);
	// 木
	std::shared_ptr<M_OpaqueSimpleLit> tree_simple_lit = std::make_shared<M_OpaqueSimpleLit>();
	Material::Initialize(tree_simple_lit, Heap.get(), desc);
	tree_simple_lit->AddTexture("../exe/assets/model/tree/T_Bark_Autumn_01_C.dds");
	// 葉
	desc.cull = D3D12_CULL_MODE_NONE;
	std::shared_ptr<M_OpaqueSimpleLit> leaf_simple_lit = std::make_shared<M_OpaqueSimpleLit>();
	Material::Initialize(leaf_simple_lit, Heap.get(), desc);
	leaf_simple_lit->AddTexture("../exe/assets/model/tree/T_Leaves_Round_01_C.dds");
	desc.cull = D3D12_CULL_MODE_BACK;

	// モデル作成
	{// スカイボックス
		std::vector<std::shared_ptr<Material>> materials;

		materials.push_back(sky_box);
	
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>(SceneBase::Layer::Environment);
		obj->SetPosition({ 0,0,0 });
		obj->SetRotation({ 0,0,0 });
		obj->SetScale({ 100,100,100 });
		std::shared_ptr<FollowActorComponent> cmp = obj->AddComponent<FollowActorComponent>(obj);
		cmp->SetFollowActor(GetRenderingEngine()->GetCamera());
		std::shared_ptr<Sphere> model = obj->AddComponent<Sphere>(obj);
		model->Create(materials);
	}
	{// グリッド
		std::vector<std::shared_ptr<Material>> materials;

		materials.push_back(opaque_depth_normal);
		materials.push_back(grid_shadow);
	
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,-1,0 });
		obj->SetRotation({ DirectX::XMConvertToRadians(90),0,0 });
		obj->SetScale({100,100,1});
		std::shared_ptr<Plane> model = obj->AddComponent<Plane>(obj);
		model->Create(materials);
	}
	{// 木1
		std::vector<std::vector<std::shared_ptr<Material>>> meshmaterials;
		std::vector<std::shared_ptr<Material>> materials;

		materials.push_back(shadow_map);
		materials.push_back(opaque_depth_normal);
		materials.push_back(custom_opaque_depth_normal);
		materials.push_back(tree_simple_lit);
		meshmaterials.push_back(materials);

		materials.clear();
		materials.push_back(leaf_shadow_map);
		materials.push_back(leaf_simple_lit);
		meshmaterials.push_back(materials);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 10,-2.5f,0 });
		obj->SetScale({2.5f,2.5f,2.5f});
		std::shared_ptr<Model> model = obj->AddComponent<Model>(obj);
		model->Create(meshmaterials, "../exe/assets/model/tree/height_tree.fbx");
	}
	{// 木2
		std::vector<std::vector<std::shared_ptr<Material>>> meshmaterials;
		std::vector<std::shared_ptr<Material>> materials;

		materials.push_back(shadow_map);
		materials.push_back(opaque_depth_normal);
		materials.push_back(custom_opaque_depth_normal);
		materials.push_back(simple_lit);
		meshmaterials.push_back(materials);

		materials.clear();
		materials.push_back(shadow_map);
		materials.push_back(simple_lit);
		meshmaterials.push_back(materials);

		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ -10,-2.5f,0 });
		obj->SetScale({ 2.5f,2.5f,2.5f });
		std::shared_ptr<Model> model = obj->AddComponent<Model>(obj);
		model->Create(meshmaterials, "../exe/assets/model/tree/height_tree.fbx");
	}
	{// 牛
		// Deffered
		std::shared_ptr<Material> deffered_albedo_normal;
		deffered_albedo_normal = std::make_shared<M_Deffered_Albedo_Normal>();
		Material::Initialize(deffered_albedo_normal,Heap.get(), desc, Material::RenderingTiming::Deffered);
		deffered_albedo_normal->AddTexture("../exe/assets/model/spot/spot_texture.png");
		
		std::vector<std::vector<std::shared_ptr<Material>>> meshmaterials;
		std::vector<std::shared_ptr<Material>> materials;

		materials.push_back(opaque_depth_normal);
		materials.push_back(deffered_albedo_normal);
		meshmaterials.push_back(materials);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,0,10 });
		std::shared_ptr<Model> model = obj->AddComponent<Model>(obj);
		model->Create(meshmaterials, "../exe/assets/model/spot/spot.fbx");
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
