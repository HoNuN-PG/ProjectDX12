#ifndef ___PRIMITIVE_H___
#define ___PRIMITIVE_H___

#include <memory>

#include "Component.h"
#include "MeshBuffer.h"
#include "Material/Material.h"

class Primitive : public Component
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
	using Component::Component;

	virtual void Init() override {};
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override;

public:
	virtual ~Primitive() {}
	virtual void Create() = 0;

protected:
	std::unique_ptr<MeshBuffer> Mesh;

public:
	Material* GetMaterial()
	{
		return MaterialData;
	}
protected:
	std::unique_ptr<MeshBuffer> ModelData;
	Material* MaterialData;

};

#endif