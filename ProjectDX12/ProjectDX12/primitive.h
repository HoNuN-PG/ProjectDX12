#ifndef ___PRIMITIVE_H___
#define ___PRIMITIVE_H___

#include <memory>

// Model
#include "RenderingComponent.h"

class Primitive : public MRenderingComponent
{
public:
	struct Vertex 
	{
		float pos[3];
		float normal[3];
		float uv[2];
		float color[4];
	};

public:
	using MRenderingComponent::MRenderingComponent;

	virtual void Init() override {};
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override;
	virtual void Rendering() override;

public:
	virtual ~Primitive() {}

public:
	virtual void Create(MeshMaterialManager::MeshMaterialSetupData materials) = 0;

protected:
	virtual void CreatePrimitive(unsigned int instanced) = 0;

};

#endif