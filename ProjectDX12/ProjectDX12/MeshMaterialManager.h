#ifndef ___MESH_MATERIAL_MANAGER_H___
#define ___MESH_MATERIAL_MANAGER_H___

#include <memory>
#include <vector>

// Material
#include "Material.h"

/**
* @class MeshMaterialManager
* @brief メッシュのマテリアルの所有者
*/
class MeshMaterialManager
{
public:
	struct MeshMaterialInfo
	{
		std::shared_ptr<Material> material; // 使用されているマテリアル
		UINT meshIdx;						// マテリアルが使用されているメッシュのインデックス
	};

public:
	void SetupMaterialsData(std::vector<std::vector<std::shared_ptr<Material>>> data);
	void BindRenderingEngine(std::weak_ptr<class GameObject> owner);

	/**
	* @brief 指定描画タイミングで描画するマテリアルを取得する
	* @param[timing] 描画タイミング
	*/
	MeshMaterialInfo GetMeshMaterial(UINT timing);

	/**
	* @brief 直前に使用したマテリアルを再使用する
	*/
	void ReuseRendering();

	void RefreshRendering();

public:
	std::vector<std::vector<std::shared_ptr<Material>>> GetMeshMaterialsData() { return MeshMaterialsData; }

private:
	std::vector<std::vector<std::shared_ptr<Material>>>	MeshMaterialsData;
	std::vector<std::vector<bool>>						UsedList;

};

#endif