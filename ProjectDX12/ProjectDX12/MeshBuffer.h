#ifndef ___MESH_BUFFER_H___
#define ___MESH_BUFFER_H___

#include <DirectXMath.h>
#include <DirectXMesh/DirectXMesh.h>
#include <vector>

// System/Rendering/Pipeline
#include "DescriptorHeap.h"
// System
#include "DirectX.h"

/// <summary>
/// メッシュバッファ
/// </summary>
class MeshBuffer
{
public:

	struct Description
	{
		const void*					pVtx;
		UINT						vtxSize;
		UINT						vtxCount;
		const void*					pIdx;
		DXGI_FORMAT					idxSize;
		UINT						idxCount;
		D3D12_PRIMITIVE_TOPOLOGY	topology;
	};

public:

	MeshBuffer() {};
	MeshBuffer(Description desc);
	virtual ~MeshBuffer();
	virtual void Draw();

protected:

	Description					Desc;
	ComPtr<ID3D12Resource>		Vtx;
	D3D12_VERTEX_BUFFER_VIEW	Vbv;
	ComPtr<ID3D12Resource>		Idx;
	D3D12_INDEX_BUFFER_VIEW		Ibv;

};

/// <summary>
/// メッシュレットバッファ
/// </summary>
class MeshletBuffer
{
public:

	struct Description
	{
		DescriptorHeap*						pHeap;
		std::vector<DirectX::XMFLOAT3>		positions;
		const void*							pVtx;
		UINT								vtxSize;
		UINT								vtxCount;
		std::vector<uint32_t>				indices;
		const void*							pIdx;
		DXGI_FORMAT							idxSize;
		UINT								idxCount;
	};

public:

	MeshletBuffer() {};
	MeshletBuffer(Description desc);
	virtual ~MeshletBuffer();
	virtual void Draw(int AmpShaderResourceStartSlot = -1, int MeshShaderResourceStartSlot = -1);

public:

	int GetMeshletCount() { return (int)meshlets.size(); }

protected:

	ComPtr<ID3D12Resource>					Vtx;
	DescriptorHeap::Handle					hVtx;

	// メッシュレットデータ
	std::vector<DirectX::Meshlet>			meshlets;
	ComPtr<ID3D12Resource>					pMeshlets;
	DescriptorHeap::Handle					hMeshlets;

	std::vector<uint8_t>					uniqueVertexIndices;
	ComPtr<ID3D12Resource>					pUniqueVertexIndices;
	DescriptorHeap::Handle					hUniqueVertexIndices;

	std::vector<DirectX::MeshletTriangle>	primitiveIndices;
	ComPtr<ID3D12Resource>					pPrimitiveIndices;
	DescriptorHeap::Handle					hPrimitiveIndices;

	// カリングデータ
	std::vector<DirectX::CullData>			cullDatas;
	ComPtr<ID3D12Resource>					pCullDatas;
	DescriptorHeap::Handle					hCullDatas;

};

#endif