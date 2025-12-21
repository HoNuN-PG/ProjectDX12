
// Model
#include "MeshMaterialManager.h"

// System/GameObject
#include "GameObject.h"

MeshMaterialManager::~MeshMaterialManager()
{
	for (auto&& item = Materials.begin(); item != Materials.end(); ++item)
	{
		std::vector<MaterialInstance> instances = item->second;
		for (int i = 0; i < instances.size(); ++i)
		{
			// マテリアルインスタンスの削除
			instances[i].first->RemoveMaterialInstance(instances[i].second);
		}
	}
}

void MeshMaterialManager::SetUp(MeshMaterialSetupData materials)
{
	for (auto&& item = materials.begin(); item != materials.end(); ++item)
	{
		for(int i = 0; i < item->second.size(); ++i)
		{ // メッシュごとにマテリアルインスタンスを作成
			MaterialInstance instance;
			instance.first = item->second[i]; // マテリアル
			instance.second = item->second[i]->AddMaterialInstance(); // マテリアルインスタンスインデックス
			Materials[item->first].push_back(instance); // メッシュごとのマテリアルインスタンスを追加
		}
	}
}

void MeshMaterialManager::Add2RenderingEngine(std::weak_ptr<class GameObject> owner)
{
	std::vector<Material::RenderingTiming> timing; // 登録済みタイミング

	for (auto&& item = Materials.begin(); item != Materials.end(); ++item)
	{
		std::vector<MaterialInstance> instances = item->second;
		for (int i = 0; i < instances.size(); ++i)
		{
			if(CheckAddedTiming(timing, instances[i].first->GetRenderTiming()))
			{ // すでに登録済みのタイミングならスキップ
				continue;
			}
			timing.push_back(instances[i].first->GetRenderTiming());
			// 登録
			owner.lock()->Add2RenderingEngine(timing.back(), instances[i].first->GetPassType());
		}
	}
}

bool MeshMaterialManager::CheckAddedTiming(std::vector<Material::RenderingTiming> timing, UINT check)
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

std::vector<MeshMaterialManager::MeshMaterialInfo> MeshMaterialManager::GetRenderingMaterial(UINT timing)
{
	std::vector<MeshMaterialInfo> infos;

	for (auto&& item = Materials.begin(); item != Materials.end(); ++item)
	{
		std::vector<MaterialInstance> instances = item->second;
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
