
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Model/Model.h"
#include "Scene/SceneManager.h"
#include "System/Rendering/ConstantBuffer/ConstantWVP.h"
#include "System/GameObject/GameObject.h"
#include "System/Rendering/RenderingEngine.h"
#include "System/StartUp.h"

void Model::Create(const char* path, MaterialRegistry::SetupTable materials)
{
	// マテリアル設定
	pMaterialRegistry = std::make_unique<MaterialRegistry>();
	pMaterialRegistry->SetUp(materials);
	
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
	if (!pScene) 
	{
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
	// 読み込んだデータから頂点バッファ生成
	MeshBuffer::Description desc = {};
	desc.vtxSize	= sizeof(Vtx);
	desc.idxSize	= DXGI_FORMAT_R16_UINT;
	desc.topology	= D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	dest.Vertices.resize(src->mNumVertices);

	// 頂点データ作成
	for (int i = 0; i < src->mNumVertices; ++i) 
	{
		// データ吸出し
		aiVector3D vtxPos		= src->mVertices[i];
		aiVector3D vtxNormal	= src->HasNormals() ? src->mNormals[i] : aiVector3D(0.0f, 0.0f, 0.0f);
		aiVector3D vtxUv		= src->HasTextureCoords(0) ? src->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);
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
	desc.pVtx		= dest.Vertices.data();
	desc.vtxCount	= dest.Vertices.size();
	desc.pIdx		= dest.Indices.data();
	desc.idxCount	= dest.Indices.size();
	pMesh.push_back(std::make_unique<MeshBuffer>(desc));
}

void Model::Draw()
{
	pMaterialRegistry->Register2RenderingEngine(Owner);
}

void Model::Rendering()
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	Material::RenderingTiming current = engine.lock()->GetCurrentRenderingTiming();
	std::vector<MaterialRegistry::MeshMaterialInfo> infos = pMaterialRegistry->GetRenderingMaterial(current); // 現在の描画タイミングで描画するマテリアルを取得

	for(auto&& info : infos)
	{
		// マテリアル設定
		info.material->WriteWVP(CalcConstantWVP::Calc3DMatrix(
			Owner.lock()->GetPosition(),
			Owner.lock()->GetRotation(),
			Owner.lock()->GetScale()),
			info.materialInstanceIdx
		);
		info.material->Bind(info.materialInstanceIdx);
		// 描画
		pMesh[info.meshIdx]->Draw();
	}
}

void MeshletModel::Create(const char* path, MaterialRegistry::SetupTable materials, DescriptorHeap* heap)
{
	// マテリアル設定
	pMaterialRegistry = std::make_unique<MaterialRegistry>();
	pMaterialRegistry->SetUp(materials);

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
	if (!pScene)
	{
		MessageBox(nullptr, importer.GetErrorString(), "Assimp Error", MB_OK);
		return;
	}

	std::vector<Mesh> meshes;
	meshes.clear();
	meshes.resize(pScene->mNumMeshes);
	for (int i = 0; i < meshes.size(); ++i)
	{
		const auto pMesh = pScene->mMeshes[i];
		CreateMesh(meshes[i], pMesh, false, false, heap);
	}
}

void MeshletModel::CreateMesh(Mesh& dest, const aiMesh* src, bool invU, bool invV, DescriptorHeap* heap)
{
	dest.Vertices.resize(src->mNumVertices);

	// 読み込んだデータから頂点バッファ生成
	MeshletBuffer::Description desc = {};
	desc.pHeap		= heap;
	desc.vtxSize	= sizeof(Vtx);
	desc.idxSize	= DXGI_FORMAT_R16_UINT;

	// 頂点データ作成
	for (int i = 0; i < src->mNumVertices; ++i)
	{
		// データ吸出し
		aiVector3D vtxPos = src->mVertices[i];
		aiVector3D vtxNormal = src->HasNormals() ? src->mNormals[i] : aiVector3D(0.0f, 0.0f, 0.0f);
		aiVector3D vtxUv = src->HasTextureCoords(0) ? src->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);
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
	for(int i = 0; i < dest.Vertices.size(); ++i) 
		desc.positions.push_back(dest.Vertices[i].pos);
	desc.pVtx = dest.Vertices.data();
	desc.vtxCount = dest.Vertices.size();

	for(int i = 0; i < dest.Indices.size(); ++i) 
		desc.indices.push_back(dest.Indices[i]);
	desc.pIdx = dest.Indices.data();
	desc.idxCount = dest.Indices.size();

	pMesh.push_back(std::make_unique<MeshletBuffer>(desc));
}

void MeshletModel::Draw()
{
	pMaterialRegistry->Register2RenderingEngine(Owner);
}

void MeshletModel::Rendering()
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	Material::RenderingTiming current = engine.lock()->GetCurrentRenderingTiming();
	std::vector<MaterialRegistry::MeshMaterialInfo> infos = pMaterialRegistry->GetRenderingMaterial(current); // 現在の描画タイミングで描画するマテリアルを取得

	for (auto&& info : infos)
	{
		// マテリアル設定
		info.material->WriteWVP(CalcConstantWVP::Calc3DMatrix(
			Owner.lock()->GetPosition(),
			Owner.lock()->GetRotation(),
			Owner.lock()->GetScale()),
			info.materialInstanceIdx
		);
		info.material->WriteMeshletCount(pMesh[info.meshIdx]->GetMeshletCount());
		info.material->Bind(info.materialInstanceIdx);
		// 描画
		pMesh[info.meshIdx]->Draw(info.material->GetAmpShaderSRVStartSlot(), info.material->GetMeshShaderSRVStartSlot());
	}
}
