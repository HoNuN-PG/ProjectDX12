#ifndef ___SPHERE_H___
#define ___SPHERE_H___

#include "primitive.h"

class Sphere : public Primitive
{
public:
	Sphere(){}
	~Sphere(){}
	void Create() override;

};

#endif