#ifndef ___PLANE_H___
#define ___PLANE_H___

#include "Model/primitive.h"

class Plane : public Primitive
{
public:

	using Primitive::Primitive;

	virtual ~Plane() {}

public:

	virtual void Create(MaterialRegistry::MeshMaterialSetupData materials) override;

private:

	virtual void CreatePrimitive(unsigned int instanced) override;

};

#endif