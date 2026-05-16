#ifndef ___MESH_MATERIAL_MANAGER_H___
#define ___MESH_MATERIAL_MANAGER_H___

#include <memory>
#include <vector>

#include "Material/Material.h"

/**
* @class MaterialRegistry
* @brief メッシュのマテリアルを管理
*/
class MaterialRegistry
{
public:

	// メッシュインデックス/メッシュに割り当てるマテリアル
	using SetupData = std::unordered_map<UINT, std::vector<std::shared_ptr<Material>>>;

	// メッシュインデックス/メッシュに割り当てるマテリアルインスタンス
	using MeshMaterialData = std::unordered_map<UINT, std::vector<Material::MaterialInstanceData>>;

public:

	// メッシュのマテリアルの情報
	struct MeshMaterialInfo
	{
		UINT meshIdx;						// メッシュインデックス
		std::shared_ptr<Material> material; // マテリアルインスタンス
		UINT materialInstanceIdx;			// マテリアルインスタンスインデックス
	};

public:

	MaterialRegistry() {}
	~MaterialRegistry();

public:

	/// <summary>
	/// セットアップ
	/// １つのメッシュに対して同じ描画タイミングのマテリアルを複数設定することはできない
	/// </summary>
	/// <param name="materials"></param>
	void SetUp(SetupData materials);

	/// <summary>
	/// レンダリングエンジンにオブジェクトを描画登録
	/// </summary>
	/// <param name="owner"></param>
	void Register2RenderingEngine(std::weak_ptr<class GameObject> owner);

private:

	/// <summary>
	/// すでに登録した描画タイミングかチェック
	/// </summary>
	/// <param name="timing">すでに登録したタイミング</param>
	/// <param name="check">チェックするタイミング</param>
	/// <returns></returns>
	bool IsRegisted(std::vector<Material::RenderingTiming> timing, UINT check);

public:

	/// <summary>
	/// メッシュが使用しているマテリアルインスタンスを取得
	/// </summary>
	/// <param name="mesh">メッシュインデックス</param>
	/// <returns></returns>
	std::vector<Material::MaterialInstanceData> GetMaterialInstances(UINT mesh) { return Materials.contains(mesh) ? Materials[mesh] : std::vector<Material::MaterialInstanceData>(); }

	/// <summary>
	/// 指定したタイミングで描画されるマテリアルインスタンスを取得
	/// </summary>
	/// <param name="timing"></param>
	/// <returns></returns>
	std::vector<MeshMaterialInfo> GetRenderingMaterial(UINT timing);

private:

	MeshMaterialData Materials;

};

#endif