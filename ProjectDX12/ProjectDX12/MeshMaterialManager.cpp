
#include "MeshMaterialManager.h"

#include "GameObject.h"

void MeshMaterialManager::SetupMaterialsData(std::vector<std::vector<std::shared_ptr<Material>>> data)
{
	MeshMaterialsData = data;
	for (int i = 0; i < MeshMaterialsData.size(); ++i)
	{
		UsedList.push_back(std::vector<bool>());
		for (int j = 0; j < MeshMaterialsData[i].size(); ++j)
		{
			MeshMaterialsData[i][j]->AddMaterialInstance();
			UsedList[i].push_back(false);
		}
	}
}

void MeshMaterialManager::BindRenderingEngine(std::weak_ptr<class GameObject> owner)
{
	for (int i = 0; i < MeshMaterialsData.size(); ++i)
	{
		for (auto material : MeshMaterialsData[i])
		{
			owner.lock()->BindRenderingEngine(material->GetRenderTiming(), material->GetPassType());
		}
	}
}

std::shared_ptr<Material> MeshMaterialManager::GetMeshMaterial(UINT timing, UINT& idx)
{
	idx = 0;
	for (int i = 0; i < MeshMaterialsData.size(); ++i)
	{
		for (int j = 0; j < MeshMaterialsData[i].size(); ++j)
		{
			// 描画タイミングが異なれば無効
			if (MeshMaterialsData[i][j]->GetRenderTiming() != timing)
			{
				continue;
			}
			// 既に使用済なら無効
			if (UsedList[i][j])
			{
				continue;
			}
			idx = i;
			UsedList[i][j] = true;
			return MeshMaterialsData[i][j];
		}
	}
	return nullptr;
}

void MeshMaterialManager::ReuseRendering()
{
	for (int i = 0; i < UsedList.size(); ++i)
	{
		for (int j = UsedList[i].size() - 1; j >= 0 ; --j)
		{
			if (UsedList[i][j])
			{
				UsedList[i][j] = false;
				break;
			}
		}
	}
}

void MeshMaterialManager::RefreshRendering()
{
	for (int i = 0; i < UsedList.size(); ++i)
	{
		for (int j = 0; j < UsedList[i].size(); ++j)
		{
			UsedList[i][j] = false;
		}
	}
}
