
// Model
#include "plane.h"

void Plane::Create(MeshMaterials materials)
{
	CreatePrimitive(0);
	// マテリアル設定
	MeshMaterialData = std::make_unique<MeshMaterialManager>();
	MeshMaterialData->SetUp(materials);
}

void Plane::Create(MeshMaterials materials, unsigned int instanced)
{
	CreatePrimitive(instanced);
	// マテリアル設定
	MeshMaterialData = std::make_unique<MeshMaterialManager>();
	MeshMaterialData->SetUp(materials);
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
