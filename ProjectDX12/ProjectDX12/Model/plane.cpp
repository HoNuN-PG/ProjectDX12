
#include "Model/plane.h"

void Plane::Create(MaterialRegistry::MeshMaterialSetupData materials)
{
	CreatePrimitive(0);
	// マテリアル設定
	pMaterialRegistry = std::make_unique<MaterialRegistry>();
	pMaterialRegistry->SetUp(materials);
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
	pMesh.push_back(std::make_unique<MeshBuffer>(desc));
}
