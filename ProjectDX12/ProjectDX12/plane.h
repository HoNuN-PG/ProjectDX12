#ifndef ___PLANE_H___
#define ___PLANE_H___

#include "primitive.h"

class Plane : public Primitive
{
public:
	virtual ~Plane() {}
	void Create() override;

};

#endif