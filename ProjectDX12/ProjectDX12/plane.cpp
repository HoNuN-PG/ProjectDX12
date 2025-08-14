
#include "plane.h"

void Plane::Create(std::vector<std::shared_ptr<Material>> materials)
{
	CreatePrimitive(0);
	MeshMaterialsData.push_back(materials);
	for (int i = 0; i < MeshMaterialsData.size(); ++i)
	{
		for (auto material : MeshMaterialsData[i])
		{
			material->SetOwner(Owner);
			material->AddMaterialInstance();
		}
	}
}

void Plane::Create(std::vector<std::shared_ptr<Material>> materials, unsigned int instanced)
{
	CreatePrimitive(instanced);
	MeshMaterialsData.push_back(materials);
	for (int i = 0; i < MeshMaterialsData.size(); ++i)
	{
		for (auto material : MeshMaterialsData[i])
		{
			material->SetOwner(Owner);
			material->AddMaterialInstance();
		}
	}
}

void Plane::CreatePrimitive(unsigned int instanced)
{
	// スクリーン頂点
	Vertex screenVtx[] =
	{
		{{-0.5f, 0.5f,0}, {0,0,-1} ,{0,0}, {1,1,1,1}} ,
		{{ 0.5f, 0.5f,0}, {0,0,-1} ,{1,0}, {1,1,1,1}} ,
		{{-0.5f,-0.5f,0}, {0,0,-1} ,{0,1}, {1,1,1,1}} ,
		{{ 0.5f,-0.5f,0}, {0,0,-1} ,{1,1}, {1,1,1,1}} ,
	};

	MeshBuffer::Description desc = {};
	// スクリーン
	desc.pVtx = screenVtx;
	desc.vtxSize = sizeof(Vertex);
	desc.vtxCount = _countof(screenVtx);
	desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	if (instanced)
	{
		bInstanced = true;
		InstanceMeshData = std::make_unique<InstanceMeshBuffer>(desc);
	}
	else
	{
		bInstanced = false;
		MeshData.push_back(std::make_unique<MeshBuffer>(desc));
	}
}
