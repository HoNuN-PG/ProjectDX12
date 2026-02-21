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
class MaterialRegistry
{
public:
	// メッシュインデックスとインデックスのメッシュに登録するマテリアル
	using SetupTable = std::unordered_map<UINT, std::vector<std::shared_ptr<Material>>>;

public:
	// マテリアルとマテリアルのインスタンスインデックス
	using MaterialInstanceData = std::pair<std::shared_ptr<Material>, UINT>;
	// メッシュインデックスとインデックスのメッシュに登録するマテリアルインスタンス
	using MeshMaterialData = std::unordered_map<UINT, std::vector<MaterialInstanceData>>;

public:
	struct MeshMaterialInfo
	{
		UINT meshIdx;						// メッシュインデックス
		std::shared_ptr<Material> material; // マテリアル
		UINT materialInstanceIdx;			// マテリアルのインスタンスインデックス
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
	void SetUp(SetupTable materials);

public:
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
	/// 指定したタイミングで描画するマテリアルを取得
	/// </summary>
	/// <param name="timing"></param>
	/// <returns></returns>
	std::vector<MeshMaterialInfo> GetRenderingMaterial(UINT timing);

	/// <summary>
	/// メッシュが使用しているマテリアルインスタンスを取得
	/// </summary>
	/// <param name="mesh">メッシュインデックス</param>
	/// <returns></returns>
	std::vector<MaterialInstanceData> GetMaterialInstances(UINT mesh) { return Materials.contains(mesh) ? Materials[mesh] : std::vector<MaterialInstanceData>(); }

private:
	MeshMaterialData Materials;

};

#endif