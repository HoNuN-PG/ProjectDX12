#ifndef ___MESH_MATERIAL_MANAGER_H___
#define ___MESH_MATERIAL_MANAGER_H___

#include <memory>
#include <vector>

// Material
#include "Material.h"

// メッシュに設定するマテリアルのセットアップデータ
using MeshMaterialSetupData = std::unordered_map<UINT, std::vector<std::shared_ptr<Material>>>; // メッシュインデックスと使用するマテリアル

/**
* @class MeshMaterialManager
* @brief メッシュのマテリアルの所有者
*/
class MeshMaterialManager
{
public:
	using MaterialInstance = std::pair<std::shared_ptr<Material>, UINT>; // 使用するマテリアルと使用するマテリアルインスタンスインデックス
	using MeshMaterial = std::unordered_map<UINT, std::vector<MaterialInstance>>; // メッシュインデックスと使用するマテリアルインスタンス

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
	/// １つのメッシュに対して同じ描画タイミングのマテリアルを複数設定することはできない（今は複数のメッシュに対しても設定できない）
	/// </summary>
	/// <param name="materials"></param>
	void SetUp(MeshMaterialSetupData materials);

public:
	/// <summary>
	/// 指定タイミングで描画するマテリアルの取得
	/// </summary>
	/// <param name="timing"></param>
	/// <returns></returns>
	std::vector<MeshMaterialInfo> GetRenderingMaterial(UINT timing);

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

public:
	std::vector<MaterialInstance> GetMaterialInstances(UINT mesh) { return Materials.contains(mesh) ? Materials[mesh] : std::vector<MaterialInstance>(); }

private:
	MeshMaterial Materials; // メッシュのマテリアル

};

#endif