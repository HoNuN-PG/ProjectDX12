#ifndef ___MESH_BUFFER_H___
#define ___MESH_BUFFER_H___

#include <DirectXMath.h>
#include <DirectXMesh/DirectXMesh.h>
#include <vector>

// System/Rendering/Pipeline
#include "DescriptorHeap.h"
// System
#include "DirectX.h"

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

class InstanceMeshBuffer : MeshBuffer
{
public:
	struct InstanceData
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 rotation;
		DirectX::XMFLOAT3 scale;

		InstanceData(
			const DirectX::XMFLOAT3& position,
			const DirectX::XMFLOAT3& rotation,
			const DirectX::XMFLOAT3& scale)
		{
			this->position = position;
			this->rotation = rotation;
			this->scale = scale;
		}
	};
	static const int MAX_INSTANCE = 100;

public:
	InstanceMeshBuffer() {};
	InstanceMeshBuffer(Description desc, unsigned int count = 1);
	virtual ~InstanceMeshBuffer();
	virtual void Draw() override;

public:
	void MappingUploder();

private:
	std::vector<InstanceData>	InsData;
	UINT						InsCount;
	ComPtr<ID3D12Resource>		InsResource;
	ComPtr<ID3D12Resource>		InsUploader;

};

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
	virtual void Draw(UINT MeshShaderSRVStartSlot);

protected:
	ComPtr<ID3D12Resource>		Vtx;
	DescriptorHeap::Handle      hVtx;

	std::vector<DirectX::Meshlet>          meshlets;
	ComPtr<ID3D12Resource>				   pMeshlets;
	DescriptorHeap::Handle				   hMeshlets;

	std::vector<uint8_t>                   uniqueVertexIndices;
	ComPtr<ID3D12Resource>				   pUniqueVertexIndices;
	DescriptorHeap::Handle				   hUniqueVertexIndices;

	std::vector<DirectX::MeshletTriangle>  primitiveIndices;
	ComPtr<ID3D12Resource>				   pPrimitiveIndices;
	DescriptorHeap::Handle				   hPrimitiveIndices;

};

#endif