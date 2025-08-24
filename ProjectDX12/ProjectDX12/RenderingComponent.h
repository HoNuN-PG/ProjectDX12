#ifndef ___RENDERING_COMPONENT_H___
#define ___RENDERING_COMPONENT_H___

#include "Component.h"

#include <memory>
#include <vector>

#include "MeshBuffer.h"

#include "Material.h"

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
	std::vector<std::shared_ptr<Material>> GetMaterials(int idx)
	{ return MeshMaterialsData[idx]; }
protected:
	std::vector<std::unique_ptr<MeshBuffer>> MeshData;
	UINT									 MeshIdx;
	std::unique_ptr<InstanceMeshBuffer>		 InstanceMeshData;
	bool									 bInstanced;
	std::vector<std::vector<std::shared_ptr<Material>>>
											 MeshMaterialsData;

};

#endif