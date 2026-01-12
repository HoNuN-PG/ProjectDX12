
// Component
#include "FollowActorComponent.h"

// Game/Camera
#include "CameraBase.h"

// Material/Materials
#include "M_Deffered_Albedo_Normal.h"
#include "M_DepthNormal.h"
#include "M_Grid.h"
#include "M_MS.h"
#include "M_Shadow.h"
#include "M_SimpleLit.h"
#include "M_SkyBox.h"

// Model
#include "Model.h"
#include "plane.h"
#include "sphere.h"

// Scene
#include "SceneSandBoxDX12.h"

// System/Constant
#include "ConstantBuffer.h"
#include "ConstantWVP.h"
// System/GameObject
#include "GameObject.h"
// System/Rendering/Pass
#include "CustomDepthNormalPass.h"
// System/Rendering/Pipeline
#include "Pipeline.h"
#include "RootSignature.h"
// System/Rendering/Volume
#include "vignette.h"
// System/Rendering
#include "GlobalResourceKey.h"

HRESULT SceneSandBoxDX12::Init()
{
	// ボリューム追加
	GetRenderingEngine()->AddVolume<Vignette>();
	// パス追加
	GetRenderingEngine()->AddRenderingPass<CustomDepthNormalPass>(
		Material::RenderingTiming::AfterOpaqueDepthNormal,RenderingPass::RenderingPassType::CustomDepthNormal);

	// マテリアル作成
	Material::Description desc;
	desc.pHeap = Heap.get();

	// ===============================
	// SkyBox
	desc.CullMode = D3D12_CULL_MODE_FRONT;
	desc.WriteDepth = TRUE;
	desc.Timing = Material::RenderingTiming::Environment;
	std::shared_ptr<M_SkyBox> sky_box = std::make_shared<M_SkyBox>();
	Material::Initialize(sky_box, desc);
	sky_box->AddTexture("../game/assets/texture/HDRI/skybox2.hdr");

	// ===============================
	// ShadowMap
	desc.CullMode = D3D12_CULL_MODE_BACK;
	desc.WriteDepth = TRUE;
	desc.Timing = Material::RenderingTiming::Shadow;
	std::shared_ptr<M_SimpleShadowMaps> shadow_map = std::make_shared<M_SimpleShadowMaps>();
	Material::Initialize(shadow_map, desc);
	// 葉シャドウマップ
	desc.CullMode = D3D12_CULL_MODE_NONE;
	desc.WriteDepth = FALSE;
	std::shared_ptr<M_OpaqueSimpleShadowMaps> leaf_shadow_map = std::make_shared<M_OpaqueSimpleShadowMaps>();
	Material::Initialize(leaf_shadow_map,  desc);
	leaf_shadow_map->AddTexture("../game/assets/model/tree/T_Leaves_Round_01_C.dds");

	// ===============================
	// DepthNormal
	desc.CullMode = D3D12_CULL_MODE_BACK;
	desc.WriteDepth = TRUE;
	desc.Timing = Material::RenderingTiming::OpaqueDepthNormal;
	std::shared_ptr<M_DepthNormal> opaque_depth_normal = std::make_shared<M_DepthNormal>();
	Material::Initialize(opaque_depth_normal, desc);

	// ===============================
	// CustomDepthNormal
	desc.CullMode = D3D12_CULL_MODE_BACK;
	desc.WriteDepth = TRUE;
	desc.Timing = Material::RenderingTiming::AfterOpaqueDepthNormal;
	desc.PassType = RenderingPass::RenderingPassType::CustomDepthNormal;
	std::shared_ptr<M_DepthNormal> custom_opaque_depth_normal = std::make_shared<M_DepthNormal>();
	Material::Initialize(custom_opaque_depth_normal, desc);

	// ===============================
	// Grid
	desc.CullMode = D3D12_CULL_MODE_BACK;
	desc.WriteDepth = TRUE;
	desc.Timing = Material::RenderingTiming::Forward;
	desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;
	std::shared_ptr<M_Grid> grid = std::make_shared<M_Grid>();
	Material::Initialize(grid, desc);
	grid->SetGridSize(1);
	grid->SetSubGridSize(5);
	// 影Grid
	std::shared_ptr<M_GridShadow> grid_shadow = std::make_shared<M_GridShadow>();
	Material::Initialize(grid_shadow, desc);
	grid_shadow->SetGridSize(1);
	grid_shadow->SetSubGridSize(5);
	// ソフト影Grid
	std::shared_ptr<M_GridShadowVSM> grid_shadow_vsm = std::make_shared<M_GridShadowVSM>();
	Material::Initialize(grid_shadow_vsm, desc);
	grid_shadow_vsm->SetGridSize(1);
	grid_shadow_vsm->SetSubGridSize(5);

	// ===============================
	// SimpleLit
	std::shared_ptr<M_SimpleLit> simple_lit = std::make_shared<M_SimpleLit>();
	Material::Initialize(simple_lit, desc);
	// 木
	std::shared_ptr<M_OpaqueSimpleLit> tree_simple_lit = std::make_shared<M_OpaqueSimpleLit>();
	Material::Initialize(tree_simple_lit, desc);
	tree_simple_lit->AddTexture("../game/assets/model/tree/T_Bark_Autumn_01_C.dds");
	// 葉
	desc.CullMode = D3D12_CULL_MODE_NONE;
	std::shared_ptr<M_OpaqueSimpleLit> leaf_simple_lit = std::make_shared<M_OpaqueSimpleLit>();
	Material::Initialize(leaf_simple_lit, desc);
	leaf_simple_lit->AddTexture("../game/assets/model/tree/T_Leaves_Round_02_C.dds");

	// ===============================
	// Meshlet
	desc.CullMode = D3D12_CULL_MODE_BACK;
	desc.WriteDepth = TRUE;
	desc.Timing = Material::RenderingTiming::Forward;
	desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;
	std::shared_ptr<M_MS> ms = std::make_shared<M_MS>();
	Material::Initialize(ms, desc);

	// ===============================
	// MeshletCulling
	desc.CullMode = D3D12_CULL_MODE_BACK;
	desc.WriteDepth = TRUE;
	desc.Timing = Material::RenderingTiming::Forward;
	desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;
	std::shared_ptr<M_MSCulling> ms_culling = std::make_shared<M_MSCulling>();
	Material::Initialize(ms_culling, desc);

	// モデル作成
	{// スカイボックス
		MeshMaterialManager::MeshMaterialSetupData materials;

		materials[0].push_back(sky_box);
	
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
		MeshMaterialManager::MeshMaterialSetupData materials;

		materials[0].push_back(opaque_depth_normal);
		materials[0].push_back(grid_shadow);
	
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,0,0 });
		obj->SetRotation({ DirectX::XMConvertToRadians(90),0,0 });
		obj->SetScale({100,100,1});
		std::shared_ptr<Plane> model = obj->AddComponent<Plane>(obj);
		model->Create(materials);
	}
	{// 木
		MeshMaterialManager::MeshMaterialSetupData materials;

		materials[0].push_back(shadow_map);
		materials[0].push_back(opaque_depth_normal);
		materials[0].push_back(custom_opaque_depth_normal);
		materials[0].push_back(tree_simple_lit);

		materials[1].push_back(leaf_shadow_map);
		materials[1].push_back(leaf_simple_lit);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,-2.5f,0 });
		obj->SetScale({2.5f,2.5f,2.5f});
		std::shared_ptr<Model> model = obj->AddComponent<Model>(obj);
		model->Create("../game/assets/model/tree/height_tree.fbx", materials);
	}
	{// 牛
		// Deffered
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Deffered;
		desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;

		std::shared_ptr<Material> deffered_albedo_normal;
		deffered_albedo_normal = std::make_shared<M_Deffered_Albedo_Normal>();
		Material::Initialize(deffered_albedo_normal, desc);
		deffered_albedo_normal->AddTexture("../game/assets/model/spot/spot_texture.png");
		
		MeshMaterialManager::MeshMaterialSetupData materials;

		materials[0].push_back(opaque_depth_normal);
		materials[0].push_back(deffered_albedo_normal);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,0,10 });
		std::shared_ptr<Model> model = obj->AddComponent<Model>(obj);
		model->Create("../game/assets/model/spot/spot.fbx", materials);
	}
#if 1
	{// 銅像
		MeshMaterialManager::MeshMaterialSetupData materials;

		materials[0].push_back(ms);

		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,0,-10 });
		std::shared_ptr<MeshletModel> model = obj->AddComponent<MeshletModel>(obj);
		model->Create("../game/assets/model/castles/staue/Staue01a.fbx", materials, Heap.get());
	}
	{// 銅像カリング
		MeshMaterialManager::MeshMaterialSetupData materials;
		
		materials[0].push_back(ms_culling);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,0,-20 });
		std::shared_ptr<MeshletModel> model = obj->AddComponent<MeshletModel>(obj);
		model->Create("../game/assets/model/castles/staue/Staue01a.fbx", materials, Heap.get());
	}
#endif

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
