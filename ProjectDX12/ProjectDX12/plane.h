#ifndef ___PLANE_H___
#define ___PLANE_H___

// Model
#include "primitive.h"

class Plane : public Primitive
{
public:
	using Primitive::Primitive;

	virtual ~Plane() {}

public:
	virtual void Create(MaterialRegistry::SetupTable materials) override;

private:
	virtual void CreatePrimitive(unsigned int instanced) override;

};

#endif