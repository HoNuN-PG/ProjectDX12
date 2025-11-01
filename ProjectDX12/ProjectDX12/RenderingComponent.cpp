
#include "GameObject.h"

#include "RenderingComponent.h"

void MeshMaterialManager::SetupMaterialsData(std::vector<std::vector<std::shared_ptr<Material>>> data)
{
	MeshMaterialsData = data;
	for (int i = 0; i < MeshMaterialsData.size(); ++i)
	{
		bUsedList.push_back(std::vector<bool>());
		for (int j = 0; j < MeshMaterialsData[i].size(); ++j)
		{
			MeshMaterialsData[i][j]->AddMaterialInstance();
			bUsedList[i].push_back(false);
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
			if (bUsedList[i][j])
			{
				continue;
			}
			idx = i;		
			bUsedList[i][j] = true;
			return MeshMaterialsData[i][j];
		}
	}
	return nullptr;
}

void MeshMaterialManager::RefreshRendering()
{
	for (int i = 0; i < bUsedList.size(); ++i)
	{
		for(int j = 0;j < bUsedList[i].size();++j)
		{
			bUsedList[i][j] = false;
		}
	}
}

void RenderingComponent::RefreshRendering()
{
	MeshMaterial->RefreshRendering();
}