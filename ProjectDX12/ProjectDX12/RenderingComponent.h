#ifndef ___RENDERING_COMPONENT_H___
#define ___RENDERING_COMPONENT_H___

#include <memory>
#include <vector>

// Model
#include "MeshMaterialManager.h"
#include "MeshBuffer.h"

// System/Component
#include "Component.h"

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
	virtual void ReuseRendering();
	virtual void RefreshRendering();

public:
	std::vector<std::shared_ptr<Material>> GetMeshMaterials(UINT meshIdx) { return MeshMaterialData->GetMaterials(meshIdx); }

protected:
	// メッシュデータ
	std::vector<std::unique_ptr<MeshBuffer>> MeshData;

	// インスタンスメッシュデータ
	bool									 bInstanced;
	std::unique_ptr<InstanceMeshBuffer>		 InstanceMeshData;

	// マテリアルデータ
	std::unique_ptr<MeshMaterialManager>	 MeshMaterialData;

};

#endif