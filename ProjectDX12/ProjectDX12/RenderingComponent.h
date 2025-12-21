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

public:
	std::vector<MeshMaterialManager::MaterialInstance> GetMeshMaterialInstances(UINT meshIdx) { return MeshMaterialData->GetMaterialInstances(meshIdx); }

protected:
	// メッシュデータ
	std::vector<std::unique_ptr<MeshBuffer>> MeshData;

	// インスタンスメッシュデータ
	bool												bInstanced;
	std::vector<std::unique_ptr<InstanceMeshBuffer>>	InstanceMeshData;

protected:
	// マテリアルデータ
	std::unique_ptr<MeshMaterialManager>	 MeshMaterialData;

};

#endif