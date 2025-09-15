
#include "Model.h"

#include "StartUp.h"
#include "SceneManager.h"
#include "GameObject.h"

#include "RenderingEngine.h"
#include "ConstantWVP.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma comment(lib, "assimp-vc141-mtd.lib")

void Model::Create(std::vector<std::vector<std::shared_ptr<Material>>> meshmaterials, const char* path)
{
	MeshIdx = 0;
	MeshMaterialsData = meshmaterials;
	for (int i = 0; i < MeshMaterialsData.size(); ++i)
	{
		for (auto material : MeshMaterialsData[i])
		{
			material->SetOwner(Owner);
			material->AddMaterialInstance();
		}
	}

	// モデル読込
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_GenSmoothNormals;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_RemoveRedundantMaterials;
	flag |= aiProcess_OptimizeMeshes;
	flag |= aiProcess_FlipUVs;
	const aiScene* pScene = importer.ReadFile(path, flag);
	if (!pScene) {
		MessageBox(nullptr, importer.GetErrorString(), "Assimp Error", MB_OK);
		return;
	}

	std::vector<Mesh> meshes;
	meshes.clear();
	meshes.resize(pScene->mNumMeshes);
	for (int i = 0; i < meshes.size(); ++i)
	{
		const auto pMesh = pScene->mMeshes[i];
		CreateMesh(meshes[i], pMesh, false,false);
	}
}

void Model::CreateMesh(Mesh& dest, const aiMesh* src, bool invU, bool invV)
{
	MeshBuffer::Description desc = {};

	// 読み込んだデータから頂点バッファ生成
	desc.vtxSize = sizeof(Vtx);
	desc.idxSize = DXGI_FORMAT_R16_UINT;
	desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	dest.Vertices.resize(src->mNumVertices);

	// 頂点データ作成
	for (int i = 0; i < src->mNumVertices; ++i) 
	{
		// データ吸出し
		aiVector3D vtxPos = src->mVertices[i];
		aiVector3D vtxNormal = src->HasNormals() ?
			src->mNormals[i] : aiVector3D(0.0f, 0.0f, 0.0f);
		aiVector3D vtxUv = src->HasTextureCoords(0) ?
			src->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);
		// 読み出したデータを設定
		dest.Vertices[i] =
		{
			DirectX::XMFLOAT3(vtxPos.x, vtxPos.y, vtxPos.z),
			DirectX::XMFLOAT3(vtxNormal.x, vtxNormal.y, vtxNormal.z),
			DirectX::XMFLOAT2(vtxUv.x, vtxUv.y),
			DirectX::XMFLOAT4(1,1,1,1),
		};
	}

	dest.Indices.resize(src->mNumFaces * 3);

	// インデックスデータ作成
	for (int i = 0; i < src->mNumFaces; ++i) 
	{
		aiFace& face = src->mFaces[i];
		dest.Indices[i * 3 + 0] = (face.mIndices[0]);
		dest.Indices[i * 3 + 1] = (face.mIndices[1]);
		dest.Indices[i * 3 + 2] = (face.mIndices[2]);
	}

	// データ生成
	desc.pVtx = dest.Vertices.data();
	desc.vtxCount = dest.Vertices.size();
	desc.pIdx = dest.Indices.data();
	desc.idxCount = dest.Indices.size();
	MeshData.push_back(std::make_unique<MeshBuffer>(desc));
}

void Model::Draw()
{
	for (int i = 0; i < MeshMaterialsData.size(); ++i)
	{
		for (auto material : MeshMaterialsData[i])
		{
			Owner.lock()->BindRenderingEngine(material->GetRenderTiming(), material->GetPassType());
		}
	}
}

void Model::Rendering()
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	Material::RenderingTiming current = engine.lock()->GetCurrentRenderingPass();
	for (auto material : MeshMaterialsData[MeshIdx])
	{
		if (material->GetRenderTiming() == current)
		{
			material->WriteWVP(ConstantWVP::Calc3DMatrix(
				Owner.lock()->GetPosition(),
				Owner.lock()->GetRotation(),
				Owner.lock()->GetScale()));
			material->Bind();
			MeshData[MeshIdx]->Draw();
			break;
		}
	}
	MeshIdx = MeshIdx + 1 >= MeshData.size() ? 0 : MeshIdx + 1;
}
