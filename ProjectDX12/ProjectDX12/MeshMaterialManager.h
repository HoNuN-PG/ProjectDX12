#ifndef ___MESH_MATERIAL_MANAGER_H___
#define ___MESH_MATERIAL_MANAGER_H___

#include <memory>
#include <vector>

// Material
#include "Material.h"

/**
* @class MeshMaterialManager
* @brief メッシュが使用するマテリアルを管理
*/
class MeshMaterialManager
{
public:
	// メッシュインデックスとインデックスのメッシュが使用するマテリアル（主にセットアップ用）
	using MeshMaterialSetupData = std::unordered_map<UINT, std::vector<std::shared_ptr<Material>>>;
	// 使用するマテリアルとマテリアルインスタンスインデックス（マテリアルインスタンス）
	using MaterialInstance = std::pair<std::shared_ptr<Material>, UINT>;
	// メッシュインデックスとインデックスのメッシュが使用するマテリアルインスタンス
	using MeshMaterial = std::unordered_map<UINT, std::vector<MaterialInstance>>;

public:
	struct MeshMaterialInfo
	{
		UINT meshIdx;						// メッシュインデックス
		std::shared_ptr<Material> material; // マテリアル
		UINT materialInstanceIdx;			// マテリアルインスタンスインデックス
	};

public:
	MeshMaterialManager() {}
	~MeshMaterialManager();

public:
	/// <summary>
	/// セットアップ
	/// １つのメッシュに対して同じ描画タイミングのマテリアルを複数設定することはできない
	/// </summary>
	/// <param name="materials"></param>
	void SetUp(MeshMaterialSetupData materials);

public:
	/// <summary>
	/// 指定したタイミングで描画するマテリアルを取得
	/// </summary>
	/// <param name="timing"></param>
	/// <returns></returns>
	std::vector<MeshMaterialInfo> GetRenderingMaterial(UINT timing);

	// メッシュが使用しているマテリアルを取得
	std::vector<MaterialInstance> GetMaterialInstances(UINT mesh) { return Materials.contains(mesh) ? Materials[mesh] : std::vector<MaterialInstance>(); }

public:
	// レンダリングエンジンにオブジェクトを描画登録
	void Add2RenderingEngine(std::weak_ptr<class GameObject> owner);

private:
	/// <summary>
	/// すでに登録した描画タイミングかチェック
	/// </summary>
	/// <param name="timing">登録したタイミング</param>
	/// <param name="check">チェックするタイミング</param>
	/// <returns></returns>
	bool CheckAddedTiming(std::vector<Material::RenderingTiming> timing, UINT check);

private:
	// 各メッシュが使用しているマテリアル
	MeshMaterial Materials;

};

#endif