#ifndef ___PRIMITIVE_H___
#define ___PRIMITIVE_H___

#include <memory>

// Model
#include "RenderingComponent.h"

class Primitive : public RenderingComponent
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
	using RenderingComponent::RenderingComponent;

	virtual void Init() override {};
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override;
	virtual void Rendering() override;

public:
	virtual ~Primitive() {}

public:
	virtual void Create(MeshMaterialSetupData materials) = 0;
	virtual void Create(MeshMaterialSetupData materials, unsigned int instanced) = 0;

protected:
	virtual void CreatePrimitive(unsigned int instanced) = 0;

};

#endif