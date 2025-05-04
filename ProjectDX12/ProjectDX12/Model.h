#ifndef ___MODEL_H___
#define ___MODEL_H___

#include <DirectXMath.h>
#include <memory>

#include "Component.h"
#include "MeshBuffer.h"
#include "Material/Material.h"

class Model : public Component
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
	using Component::Component;

	virtual void Init() override {};
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override;

public:
	virtual ~Model(){}
	void Create(Material* material, const char* path);

public:
	Material* GetMaterial()
	{ return MaterialData; }
protected:
	std::unique_ptr<MeshBuffer> ModelData;
	Material* MaterialData;

};

#endif