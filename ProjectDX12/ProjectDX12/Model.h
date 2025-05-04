#ifndef ___MODEL_H___
#define ___MODEL_H___

#include <DirectXMath.h>
#include <memory>

#include "MeshBuffer.h"
#include "Material/Material.h"

class Model
{
public:
	struct ModelVertex 
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT4 color;
	};

public:
	Model() {};
	virtual ~Model(){}
	void Create(Material* material, const char* path);
	void Draw();

public:
	Material* GetMaterial()
	{ return MaterialData; }
protected:
	std::unique_ptr<MeshBuffer> ModelData;
	Material* MaterialData;

};

#endif