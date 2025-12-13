#ifndef ___MODEL_H___
#define ___MODEL_H___

#include <DirectXMath.h>
#include <memory>

// Model
#include "RenderingComponent.h"

struct aiMesh;

class Model : public RenderingComponent
{
public:
	struct Vtx
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT4 color;
	};

	struct Mesh
	{
		std::vector<Vtx> Vertices;
		std::vector<WORD> Indices;
	};

public:
	using RenderingComponent::RenderingComponent;

	virtual void Init() override {};
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override;
	virtual void Rendering() override;

public:
	virtual ~Model(){}

public:
	void Create(const char* path, MeshMaterials materials);

private:
	void CreateMesh(Mesh& dest, const aiMesh* src, bool invU, bool invV);

};

#endif