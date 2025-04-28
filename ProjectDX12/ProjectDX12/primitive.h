#ifndef ___PRIMITIVE_H___
#define ___PRIMITIVE_H___

#include <memory>

#include "MeshBuffer.h"

class Primitive
{
public:
	struct Vertex 
	{
		float pos[3];
		float uv[2];
		float color[4];
	};

public:
	Primitive(){}
	~Primitive(){}
	virtual void Create() = 0;
	virtual void Draw();

protected:
	std::unique_ptr<MeshBuffer> Mesh;

};

#endif