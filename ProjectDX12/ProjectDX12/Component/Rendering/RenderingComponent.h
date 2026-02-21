#ifndef ___RENDERING_COMPONENT_H___
#define ___RENDERING_COMPONENT_H___

#include <memory>
#include <vector>

#include "Model/MaterialRegistry.h"
#include "Model/MeshBuffer.h"
#include "System/Component/Component.h"

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
	virtual std::vector<MaterialRegistry::MaterialInstanceData> GetMeshMaterialInstances(UINT meshIdx) { return pMaterialRegistry->GetMaterialInstances(meshIdx); }

protected:
	// マテリアルデータ
	std::unique_ptr<MaterialRegistry> pMaterialRegistry;

};

/// <summary>
/// メッシュ描画コンポーネント
/// </summary>
class MRenderingComponent : public RenderingComponent
{
public:
	using RenderingComponent::RenderingComponent;

	virtual ~MRenderingComponent() {};

protected:
	// メッシュデータ
	std::vector<std::unique_ptr<MeshBuffer>> pMesh;

};

/// <summary>
/// メッシュレット描画コンポーネント
/// </summary>
class MLRenderingComponent : public RenderingComponent
{
public:
	using RenderingComponent::RenderingComponent;

	virtual ~MLRenderingComponent() {};

public:
	std::vector<MaterialRegistry::MaterialInstanceData> GetMeshMaterialInstances(UINT meshIdx) { return pMaterialRegistry->GetMaterialInstances(meshIdx); }

protected:
	// メッシュデータ
	std::vector<std::unique_ptr<MeshletBuffer>> pMesh;

};

#endif