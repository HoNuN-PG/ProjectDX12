#ifndef ___RENDERING_COMPONENT_H___
#define ___RENDERING_COMPONENT_H___

#include <memory>
#include <vector>

#include "Material.h"

#include "MeshBuffer.h"

#include "Component.h"

/// <summary>
/// メッシュが持つマテリアルの管理
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

private:

};

class RenderingComponent : public Component
{
public:
	using Component::Component;

	virtual ~RenderingComponent() {};

	virtual void Init() override {};
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override {}
	virtual void Rendering() = 0;
	virtual void RefreshRendering();

public:
	std::vector<std::shared_ptr<Material>> GetMeshMaterials(int meshIdx) { return MeshMaterial->GetMeshMaterialsData()[meshIdx]; }

protected:
	// メッシュデータ
	std::vector<std::unique_ptr<MeshBuffer>> MeshData;

	// インスタンスメッシュデータ
	bool									 bInstanced;
	std::unique_ptr<InstanceMeshBuffer>		 InstanceMeshData;

	// メッシュがもつマテリアル
	std::unique_ptr<MeshMaterialManager>	 MeshMaterial;

};

#endif