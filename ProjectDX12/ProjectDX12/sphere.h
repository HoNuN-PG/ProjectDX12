#ifndef ___SPHERE_H___
#define ___SPHERE_H___

// Model
#include "primitive.h"

class Sphere : public Primitive
{
public:
	using Primitive::Primitive;

	virtual ~Sphere(){}

public:
	virtual void Create(MeshMaterialSetupData materials) override;

private:
	virtual void CreatePrimitive(unsigned int instanced) override;

};

#endif