
// Model
#include "MeshMaterialManager.h"

// System/GameObject
#include "GameObject.h"

void MeshMaterialManager::SetUp(MeshMaterials materials)
{
	Materials = materials;
	for(auto&& item = Materials.begin();item != Materials.end(); ++item)
	{
		for (int i = 0; i < item->second.size(); ++i)
		{
			// マテリアルインスタンス追加
			item->second[i]->AddMaterialInstance();

			// 使用状況初期化
			Usage[item->first].push_back(false);
		}
	}
}

void MeshMaterialManager::Register2RenderingEngine(std::weak_ptr<class GameObject> owner)
{
	for (auto&& item = Materials.begin(); item != Materials.end(); ++item)
	{
		for (int i = 0; i < item->second.size(); ++i)
		{
			owner.lock()->Add2RenderingEngine(item->second[i]->GetRenderTiming(), item->second[i]->GetPassType());
		}
	}
}

MeshMaterialManager::MeshMaterialInfo MeshMaterialManager::GetRenderingMaterial(UINT timing)
{
	for (auto&& item = Materials.begin(); item != Materials.end(); ++item)
	{
		for (int i = 0; i < item->second.size(); ++i)
		{
			// 描画タイミングが異なれば無効
			if (item->second[i]->GetRenderTiming() != timing)
			{
				continue;
			}
			// 既に使用済なら無効
			if (Usage[item->first][i])
			{
				continue;
			}
			// 使用状況を設定して描画するマテリアルを返す
			Usage[item->first][i] = true;
			MeshMaterialInfo info;
			info.material = item->second[i];
			info.meshIdx = i;

			return info;
		}
	}
	return MeshMaterialInfo();
}

void MeshMaterialManager::Reuse()
{
	for (auto&& item = Usage.begin(); item != Usage.end(); ++item)
	{
		for (int i = item->second.size() - 1; i << item->second.size() >= 0; --i)
		{
			if (item->second[i])
			{
				item->second[i] = false;
				break;
			}
		}
	}
}

void MeshMaterialManager::Refresh()
{
	for (auto&& item = Usage.begin(); item != Usage.end(); ++item)
	{
		for (int i = item->second.size() - 1; i << item->second.size() >= 0; --i)
		{
			item->second[i] = false;
		}
	}
}
