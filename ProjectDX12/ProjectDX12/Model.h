#ifndef ___MODEL_H___
#define ___MODEL_H___

#include <DirectXMath.h>
#include <memory>

// Model
#include "RenderingComponent.h"

struct aiMesh;

class Model : public VSRenderingComponent
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
	using VSRenderingComponent::VSRenderingComponent;

	virtual void Init() override {};
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override;
	virtual void Rendering() override;

public:
	virtual ~Model(){}

public:
	void Create(const char* path, MeshMaterialSetupData materials);

private:
	void CreateMesh(Mesh& dest, const aiMesh* src, bool invU, bool invV);

};

class MeshletModel : public MSRenderingComponent
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
		std::vector<Vtx>		Vertices;
		std::vector<uint32_t>	Indices;
	};

public:
	using MSRenderingComponent::MSRenderingComponent;

	virtual void Init() override {};
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override;
	virtual void Rendering() override;

public:
	virtual ~MeshletModel() {}

public:
	void Create(const char* path, MeshMaterialSetupData materials, DescriptorHeap* heap);

private:
	void CreateMesh(Mesh& dest, const aiMesh* src, bool invU, bool invV, DescriptorHeap* heap);

};

#endif