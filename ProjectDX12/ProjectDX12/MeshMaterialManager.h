#ifndef ___MESH_MATERIAL_MANAGER_H___
#define ___MESH_MATERIAL_MANAGER_H___

#include <memory>
#include <vector>

#include "Material.h"

/// <summary>
/// メッシュのマテリアルを管理
/// </summary>
class MeshMaterialManager
{
public:
	void SetupMaterialsData(std::vector<std::vector<std::shared_ptr<Material>>> data);
	void BindRenderingEngine(std::weak_ptr<class GameObject> owner);
	std::shared_ptr<Material> GetMeshMaterial(UINT timing, UINT& idx);
	void RefreshRendering();

public:
	std::vector<std::vector<std::shared_ptr<Material>>> GetMeshMaterialsData() { return MeshMaterialsData; }

private:
	std::vector<std::vector<std::shared_ptr<Material>>>	MeshMaterialsData;
	std::vector<std::vector<bool>>						bUsedList;

};

#endif