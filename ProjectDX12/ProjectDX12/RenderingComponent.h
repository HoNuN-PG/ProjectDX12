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
	virtual std::vector<MeshMaterialManager::MaterialInstance> GetMeshMaterialInstances(UINT meshIdx) { return MeshMaterialData->GetMaterialInstances(meshIdx); }

protected:
	// マテリアルデータ
	std::unique_ptr<MeshMaterialManager> MeshMaterialData;

};

class VSRenderingComponent : public RenderingComponent
{
public:
	using RenderingComponent::RenderingComponent;

	virtual ~VSRenderingComponent() {};

protected:
	// メッシュデータ
	std::vector<std::unique_ptr<MeshBuffer>> MeshData;

};

class MSRenderingComponent : public RenderingComponent
{
public:
	using RenderingComponent::RenderingComponent;

	virtual ~MSRenderingComponent() {};

public:
	std::vector<MeshMaterialManager::MaterialInstance> GetMeshMaterialInstances(UINT meshIdx) { return MeshMaterialData->GetMaterialInstances(meshIdx); }

protected:
	// メッシュデータ
	std::vector<std::unique_ptr<MeshletBuffer>> MeshData;

};

#endif