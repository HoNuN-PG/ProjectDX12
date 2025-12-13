#ifndef ___MESH_MATERIAL_MANAGER_H___
#define ___MESH_MATERIAL_MANAGER_H___

#include <memory>
#include <vector>

// Material
#include "Material.h"

// メッシュに設定するマテリアル
using MeshMaterials = std::unordered_map<UINT, std::vector<std::shared_ptr<Material>>>;

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
		UINT meshIdx;						// メッシュのインデックス
	};

public:
	// セットアップ
	void SetUp(MeshMaterials materials);

	// レンダリングエンジンに参照を登録
	void Register2RenderingEngine(std::weak_ptr<class GameObject> owner);

public:
	/// <summary>
	/// 指定タイミングで描画するマテリアルの取得
	/// </summary>
	/// <param name="timing"></param>
	/// <returns></returns>
	MeshMaterialInfo GetRenderingMaterial(UINT timing);

public:
	// 直前に使用したマテリアルの使用状況をリセット
	void Reuse();

	// リフレッシュ
	void Refresh();

public:
	std::vector<std::shared_ptr<Material>> GetMaterials(UINT mesh) { Materials.find(mesh); }

private:
	std::unordered_map<UINT, std::vector<std::shared_ptr<Material>>>	Materials;	// メッシュごとのマテリアル
	std::unordered_map<UINT, std::vector<bool>>							Usage;		// メッシュごとのマテリアルの使用状況

};

#endif