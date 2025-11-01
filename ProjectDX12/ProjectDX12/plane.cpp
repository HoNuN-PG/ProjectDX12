
#include "plane.h"

void Plane::Create(std::vector<std::shared_ptr<Material>> materials)
{
	CreatePrimitive(0);
	// マテリアル設定
	MeshMaterial = std::make_unique<MeshMaterialManager>();
	std::vector<std::vector<std::shared_ptr<Material>>> data; 
	data.push_back(materials);
	MeshMaterial->SetupMaterialsData(data);
}

void Plane::Create(std::vector<std::shared_ptr<Material>> materials, unsigned int instanced)
{
	CreatePrimitive(instanced);
	// マテリアル設定
	MeshMaterial = std::make_unique<MeshMaterialManager>();
	std::vector<std::vector<std::shared_ptr<Material>>> data;
	data.push_back(materials);
	MeshMaterial->SetupMaterialsData(data);
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
