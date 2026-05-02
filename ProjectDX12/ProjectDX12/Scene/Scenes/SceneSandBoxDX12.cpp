
#include "Component/FollowActorComponent.h"
#include "GameObject/Camera/CameraBase.h"
#include "Material/Materials/M_Deffered_Albedo.h"
#include "Material/Materials/M_DepthNormal.h"
#include "Material/Materials/M_Grid.h"
#include "Material/Materials/M_MS.h"
#include "Material/Materials/M_Shadow.h"
#include "Material/Materials/M_SimpleLit.h"
#include "Material/Materials/M_SkyBox.h"
#include "Model/Model.h"
#include "Model/plane.h"
#include "Model/sphere.h"
#include "Scene/Scenes/SceneSandBoxDX12.h"
#include "System/Rendering/ConstantBuffer/ConstantBuffer.h"
#include "System/Rendering/ConstantBuffer/ConstantWVP.h"
#include "System/GameObject/GameObject.h"
#include "System/Rendering/Pass/CustomDepthNormalPass.h"
#include "System/Rendering/Pipeline/PipelineState.h"
#include "System/Rendering/Pipeline/RootSignature.h"
#include "System/Rendering/Volume/vignette.h"
#include "System/Rendering/GlobalResourceKey.h"

HRESULT SceneSandBoxDX12::Init()
{
	// ボリューム追加
	GetRenderingEngine()->AddVolume<Vignette>();
	// パス追加
	GetRenderingEngine()->AddRenderingPass<CustomDepthNormalPass>(
		Material::RenderingTiming::AfterOpaqueDepthNormal,RenderingPass::RenderingPassType::CustomDepthNormal);

	// マテリアル作成
	Material::Description desc;
	desc.pHeap = pHeap.get();

	// SkyBox
	std::shared_ptr<M_SkyBox> sky_box = std::make_shared<M_SkyBox>();
	{
		desc.CullMode = D3D12_CULL_MODE_FRONT;
		desc.WriteDepth = FALSE;
		desc.Timing = Material::RenderingTiming::Environment;
		
		Material::Initialize(sky_box, desc);
		sky_box->AddTexture("../game/assets/texture/HDRI/skybox2.hdr");
	}
	// ShadowMap
	std::shared_ptr<M_SimpleShadowMaps> shadow_map = std::make_shared<M_SimpleShadowMaps>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Shadow;

		Material::Initialize(shadow_map, desc);
	}
	// LeafShadowMap
	std::shared_ptr<M_OpaqueSimpleShadowMaps> leaf_shadow_map = std::make_shared<M_OpaqueSimpleShadowMaps>();
	{
		desc.CullMode = D3D12_CULL_MODE_NONE;
		desc.WriteDepth = FALSE;

		Material::Initialize(leaf_shadow_map, desc);
		leaf_shadow_map->AddTexture("../game/assets/model/tree/T_Leaves_Round_01_C.dds");
	}
	// DepthNormal
	std::shared_ptr<M_DepthNormal> opaque_depth_normal = std::make_shared<M_DepthNormal>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::OpaqueDepthNormal;

		Material::Initialize(opaque_depth_normal, desc);
	}
	// CustomDepthNormal
	std::shared_ptr<M_DepthNormal> custom_opaque_depth_normal = std::make_shared<M_DepthNormal>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::AfterOpaqueDepthNormal;
		desc.PassType = RenderingPass::RenderingPassType::CustomDepthNormal;

		Material::Initialize(custom_opaque_depth_normal, desc);
	}
	// Grid
	std::shared_ptr<M_Grid> grid = std::make_shared<M_Grid>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Forward;
		desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;

		Material::Initialize(grid, desc);
		grid->SetGridSize(1);
		grid->SetSubGridSize(5);
	}
	// ShadowGrid
	std::shared_ptr<M_GridShadow> grid_shadow = std::make_shared<M_GridShadow>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Forward;
		desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;

		Material::Initialize(grid_shadow, desc);
		grid_shadow->SetGridSize(1);
		grid_shadow->SetSubGridSize(5);
	}
	// SoftShadowGrid
	std::shared_ptr<M_GridShadowVSM> grid_shadow_vsm = std::make_shared<M_GridShadowVSM>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Forward;
		desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;

		Material::Initialize(grid_shadow_vsm, desc);
		grid_shadow_vsm->SetGridSize(1);
		grid_shadow_vsm->SetSubGridSize(5);
	}
	// Deffered
	std::shared_ptr<Material> deffered_albedo_normal = std::make_shared<M_Deffered_Albedo>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Deffered;
		desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;

		Material::Initialize(deffered_albedo_normal, desc);
		deffered_albedo_normal->AddTexture("../game/assets/model/spot/spot_texture.png");
	}
	// SimpleLit
	std::shared_ptr<M_SimpleLit> simple_lit = std::make_shared<M_SimpleLit>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Forward;
		desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;

		Material::Initialize(simple_lit, desc);
	}
	// Tree
	std::shared_ptr<M_OpaqueSimpleLit> tree_simple_lit = std::make_shared<M_OpaqueSimpleLit>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Forward;
		desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;

		Material::Initialize(tree_simple_lit, desc);
		tree_simple_lit->AddTexture("../game/assets/model/tree/T_Bark_Autumn_01_C.dds");
	}
	// Leaf
	std::shared_ptr<M_OpaqueSimpleLit> leaf_simple_lit = std::make_shared<M_OpaqueSimpleLit>();
	{
		desc.CullMode = D3D12_CULL_MODE_NONE;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Forward;
		desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;

		Material::Initialize(leaf_simple_lit, desc);
		leaf_simple_lit->AddTexture("../game/assets/model/tree/T_Leaves_Round_02_C.dds");
	}
	// Meshlet
	std::shared_ptr<M_MS> ms = std::make_shared<M_MS>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Forward;
		desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;

		Material::Initialize(ms, desc);
	}
	// MeshletCulling
	std::shared_ptr<M_MSCulling> ms_culling = std::make_shared<M_MSCulling>();
	{
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.WriteDepth = TRUE;
		desc.Timing = Material::RenderingTiming::Forward;
		desc.PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;

		Material::Initialize(ms_culling, desc);
	}

	// モデル作成
	{
		// スカイボックス
		MaterialRegistry::MeshMaterialSetupData materials;

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
	{
		// グリッド
		MaterialRegistry::MeshMaterialSetupData materials;

		materials[0].push_back(opaque_depth_normal);
		materials[0].push_back(grid_shadow);
	
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,0,0 });
		obj->SetRotation({ DirectX::XMConvertToRadians(90),0,0 });
		obj->SetScale({100,100,1});
		std::shared_ptr<Plane> model = obj->AddComponent<Plane>(obj);
		model->Create(materials);
	}
	{
		// 木
		MaterialRegistry::MeshMaterialSetupData materials;

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
	{
		// 牛
		MaterialRegistry::MeshMaterialSetupData materials;

		materials[0].push_back(opaque_depth_normal);
		materials[0].push_back(deffered_albedo_normal);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,0,10 });
		std::shared_ptr<Model> model = obj->AddComponent<Model>(obj);
		model->Create("../game/assets/model/spot/spot.fbx", materials);
	}
#if 1
	{
		// 銅像
		MaterialRegistry::MeshMaterialSetupData materials;

		materials[0].push_back(ms);

		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,0,-10 });
		std::shared_ptr<MeshletModel> model = obj->AddComponent<MeshletModel>(obj);
		model->Create("../game/assets/model/castles/staue/Staue01a.fbx", materials, pHeap.get());
	}
	{
		// 銅像カリング
		MaterialRegistry::MeshMaterialSetupData materials;
		
		materials[0].push_back(ms_culling);
		
		std::shared_ptr<GameObject> obj = AddGameObject<GameObject>();
		obj->SetPosition({ 0,0,-20 });
		std::shared_ptr<MeshletModel> model = obj->AddComponent<MeshletModel>(obj);
		model->Create("../game/assets/model/castles/staue/Staue01a.fbx", materials, pHeap.get());
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
