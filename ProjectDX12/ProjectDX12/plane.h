#ifndef ___PLANE_H___
#define ___PLANE_H___

// Model
#include "primitive.h"

class Plane : public Primitive
{
public:
	using Primitive::Primitive;

	virtual ~Plane() {}
	virtual void Create(std::vector<std::shared_ptr<Material>> materials) override;
	virtual void Create(std::vector<std::shared_ptr<Material>> materials, unsigned int instanced) override;

private:
	virtual void CreatePrimitive(unsigned int instanced) override;

};

#endif