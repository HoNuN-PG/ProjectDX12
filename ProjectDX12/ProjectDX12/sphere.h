#ifndef ___SPHERE_H___
#define ___SPHERE_H___

// Model
#include "primitive.h"

class Sphere : public Primitive
{
public:
	using Primitive::Primitive;

	virtual ~Sphere(){}
	virtual void Create(std::vector<std::shared_ptr<Material>> materials) override;
	virtual void Create(std::vector<std::shared_ptr<Material>> materials, unsigned int instanced) override;

private:
	virtual void CreatePrimitive(unsigned int instanced) override;

};

#endif