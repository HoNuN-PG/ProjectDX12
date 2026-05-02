#ifndef ___SPHERE_H___
#define ___SPHERE_H___

#include "Model/primitive.h"

class Sphere : public Primitive
{
public:

	using Primitive::Primitive;

	virtual ~Sphere(){}

public:

	virtual void Create(MaterialRegistry::MeshMaterialSetupData materials) override;

private:

	virtual void CreatePrimitive(unsigned int instanced) override;

};

#endif