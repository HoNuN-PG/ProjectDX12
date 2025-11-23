#ifndef ___RENDERING_COMPONENT_H___
#define ___RENDERING_COMPONENT_H___

#include <memory>
#include <vector>

#include "MeshMaterialManager.h"

#include "MeshBuffer.h"

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
	std::vector<std::shared_ptr<Material>> GetMeshMaterials(int meshIdx) { return MeshMaterial->GetMeshMaterialsData()[meshIdx]; }

protected:
	// メッシュデータ
	std::vector<std::unique_ptr<MeshBuffer>> MeshData;

	// インスタンスメッシュデータ
	bool									 bInstanced;
	std::unique_ptr<InstanceMeshBuffer>		 InstanceMeshData;

	// メッシュのマテリアル所有者
	std::unique_ptr<MeshMaterialManager>	 MeshMaterial;

};

#endif