
#include "Model.h"
#include "StartUp.h"

#include "GameObject.h"
#include "ConstantWVP.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma comment(lib, "assimp-vc141-mtd.lib")

void Model::Create(Material* material, const char* path)
{
	MaterialData = material;

	MeshBuffer::Description desc = {};
	// モデル読込
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_JoinIdenticalVertices;
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_FlipUVs; // UV修正
	const aiScene* pScene = importer.ReadFile(path, flag);
	if (!pScene) {
		MessageBox(nullptr, importer.GetErrorString(), "Assimp Error", MB_OK);
		return;
	}

	// 読み込んだデータから頂点バッファ生成
	desc.vtxSize = sizeof(ModelVertex);
	desc.idxSize = DXGI_FORMAT_R16_UINT;
	desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	for (int i = 0; i < pScene->mNumMeshes; ++i) {
		// 頂点データの初期値作成
		std::vector<ModelVertex> modelVtx;
		for (int j = 0; j < pScene->mMeshes[i]->mNumVertices; ++j) {
			// データ吸出し
			aiVector3D vtxPos = pScene->mMeshes[i]->mVertices[j];
			aiVector3D vtxNormal = pScene->mMeshes[i]->HasNormals() ?
				pScene->mMeshes[i]->mNormals[j] : aiVector3D(0.0f, 0.0f, 0.0f);
			aiVector3D vtxUv = pScene->mMeshes[i]->HasTextureCoords(0) ?
				pScene->mMeshes[i]->mTextureCoords[0][j] : aiVector3D(0.0f, 0.0f, 0.0f);
			// 読み出したデータを設定
			modelVtx.push_back
			({
				DirectX::XMFLOAT3(vtxPos.x, vtxPos.y, vtxPos.z),
				DirectX::XMFLOAT3(vtxNormal.x, vtxNormal.y, vtxNormal.z),
				DirectX::XMFLOAT2(vtxUv.x, vtxUv.y),
				DirectX::XMFLOAT4(1,1,1,1),
			});
		}
		// インデックスデータの初期値作成
		std::vector<WORD> modelIdx;
		for (int j = 0; j < pScene->mMeshes[i]->mNumFaces; ++j) {
			aiFace& face = pScene->mMeshes[i]->mFaces[j];
			modelIdx.push_back(face.mIndices[0]);
			modelIdx.push_back(face.mIndices[1]);
			modelIdx.push_back(face.mIndices[2]);
		}
		// データ生成
		desc.pVtx = modelVtx.data();
		desc.vtxCount = modelVtx.size();
		desc.pIdx = modelIdx.data();
		desc.idxCount = modelIdx.size();
		ModelData = std::make_unique<MeshBuffer>(desc);
	}
}

void Model::Draw()
{
	// WVPの設定
	MaterialData->WriteWVP(ConstantWVP::Calc3DMatrix(
		Owner->GetPosition() ,
		Owner->GetRotation(),
		Owner->GetScale()));
	MaterialData->Draw();
	ModelData->Draw();
}
