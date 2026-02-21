
#include "Model/MaterialRegistry.h"
#include "System/GameObject/GameObject.h"

MaterialRegistry::~MaterialRegistry()
{
	for (auto&& item = Materials.begin(); item != Materials.end(); ++item)
	{
		std::vector<MaterialInstanceData> instances = item->second;
		for (int i = 0; i < instances.size(); ++i)
		{
			// マテリアルインスタンスの削除
			instances[i].first->RemoveMaterialInstance(instances[i].second);
		}
	}
}

void MaterialRegistry::SetUp(SetupTable materials)
{
	for (auto&& item = materials.begin(); item != materials.end(); ++item)
	{
		for(int i = 0; i < item->second.size(); ++i)
		{ 
			// メッシュごとにマテリアルインスタンスを作成
			MaterialInstanceData instance;
			instance.first = item->second[i]; // マテリアル
			instance.second = item->second[i]->AddMaterialInstance(); // マテリアルインスタンスインデックス
			Materials[item->first].push_back(instance); // メッシュごとに使用するマテリアルインスタンスを追加
		}
	}
}

std::vector<MaterialRegistry::MeshMaterialInfo> MaterialRegistry::GetRenderingMaterial(UINT timing)
{
	std::vector<MeshMaterialInfo> infos;

	for (auto&& item = Materials.begin(); item != Materials.end(); ++item)
	{
		std::vector<MaterialInstanceData> instances = item->second;
		for (int i = 0; i < instances.size(); ++i)
		{
			// 描画タイミングが異なれば無効
			if (instances[i].first->GetRenderTiming() != timing)
			{
				continue;
			}

			// 構造体に詰める
			MeshMaterialInfo info;
			info.meshIdx = item->first;
			info.material = instances[i].first;
			info.materialInstanceIdx = instances[i].second;
			infos.push_back(info);
		}
	}

	return infos;
}

void MaterialRegistry::Register2RenderingEngine(std::weak_ptr<class GameObject> owner)
{
	// 登録済み描画タイミング
	std::vector<Material::RenderingTiming> timing;

	for (auto&& item = Materials.begin(); item != Materials.end(); ++item)
	{
		std::vector<MaterialInstanceData> instances = item->second;
		for (int i = 0; i < instances.size(); ++i)
		{
			if(IsRegisted(timing, instances[i].first->GetRenderTiming()))
			{ 
				// すでに登録済みの描画タイミングなら無効
				continue;
			}
			// 登録
			timing.push_back(instances[i].first->GetRenderTiming());
			owner.lock()->Add2RenderingEngine(timing.back(), instances[i].first->GetPassType());
		}
	}
}

bool MaterialRegistry::IsRegisted(std::vector<Material::RenderingTiming> timing, UINT check)
{
	for(int i = 0; i < timing.size(); ++i)
	{
		if (timing[i] == check)
		{
			return true;
		}
	}
	return false;
}
