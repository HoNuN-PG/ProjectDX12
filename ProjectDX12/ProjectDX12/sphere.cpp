
#include <DirectXMath.h>

#include "sphere.h"

void Sphere::Create(std::vector<std::shared_ptr<Material>> materials)
{
	CreatePrimitive(0);
	// マテリアル設定
	MeshMaterial = std::make_unique<MeshMaterialManager>();
	std::vector<std::vector<std::shared_ptr<Material>>> data;
	data.push_back(materials);
	MeshMaterial->SetupMaterialsData(data);
}

void Sphere::Create(std::vector<std::shared_ptr<Material>> materials, unsigned int instanced)
{
	CreatePrimitive(instanced);
	// マテリアル設定
	MeshMaterial = std::make_unique<MeshMaterialManager>();
	std::vector<std::vector<std::shared_ptr<Material>>> data;
	data.push_back(materials);
	MeshMaterial->SetupMaterialsData(data);
}

void Sphere::CreatePrimitive(unsigned int instanced)
{
	// 天球作成時の定数
	const int SphereDetail = 12;
	const int SphereVtxXNum = SphereDetail * 4 + 1;					// 横方向の頂点数
	const int SphereVtxYNum = SphereDetail * 2 + 1;					// 縦方向の頂点数
	const int SphereVtxNum = SphereVtxXNum * SphereVtxYNum;			// 全体の頂点数
	const int SphereIdxXNum = SphereVtxXNum - 1;					// 横方向の面数
	const int SphereIdxYNum = SphereVtxYNum - 1;					// 縦方向の面数
	const int SphereIdxNum = SphereIdxXNum * SphereIdxYNum * 6;		// 全体のインデックス数(面 = 3+3)
	// 頂点作成
	Vertex sphereVtx[SphereVtxNum];
	for (int j = 0; j < SphereVtxYNum; ++j)
	{
		float v = (float)j / SphereIdxYNum;
		float radY = DirectX::XM_PI * v;
		float sY = sinf(radY);
		float cY = cosf(radY);
		int idx = SphereVtxXNum * j;
		for (int i = 0; i < SphereVtxXNum; ++i)
		{
			float u = (float)i / SphereIdxXNum;
			float radXZ = DirectX::XM_2PI * u;
			float x = sinf(radXZ) * sY;
			float z = cosf(radXZ) * sY;
			sphereVtx[idx + i] = { {x, cY, z}, {x, cY, z}, { u, v }, {1, 1, 1, 1} };
		}
	}
	// インデックス作成
	unsigned short sphereIdx[SphereIdxNum];
	for (int j = 0; j < SphereIdxYNum; ++j)
	{
		int vtxIdxY = j * SphereVtxXNum;
		int idxY = j * SphereIdxXNum;
		for (int i = 0; i < SphereIdxXNum; ++i)
		{
			int vtxIdx = vtxIdxY + i;
			int idx = (idxY + i) * 6;
			sphereIdx[idx + 0] = vtxIdx + 1;
			sphereIdx[idx + 1] = vtxIdx;
			sphereIdx[idx + 2] = vtxIdx + SphereVtxXNum;
			sphereIdx[idx + 3] = sphereIdx[idx + 2];
			sphereIdx[idx + 4] = sphereIdx[idx + 2] + 1;
			sphereIdx[idx + 5] = sphereIdx[idx + 0];
		}
	}
	// メッシュ作成
	MeshBuffer::Description desc = {};
	desc.pVtx = sphereVtx;
	desc.vtxSize = sizeof(Vertex);
	desc.vtxCount = SphereVtxNum;
	desc.pIdx = sphereIdx;
	desc.idxSize = DXGI_FORMAT_R16_UINT;
	desc.idxCount = SphereIdxNum;
	desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
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
