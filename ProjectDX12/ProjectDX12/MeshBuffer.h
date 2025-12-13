#ifndef ___MESH_BUFFER_H___
#define ___MESH_BUFFER_H___

#include <DirectXMath.h>
#include <vector>

// System
#include "DirectX.h"

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
	static const int MAX_INSTANCE = 100;

public:
	InstanceMeshBuffer() {};
	InstanceMeshBuffer(Description desc,unsigned int count = 0);
	virtual ~InstanceMeshBuffer();
	virtual void Draw() override;

public:
	void MappingUploder();

private:
	ComPtr<ID3D12Resource>		Ins;
	ComPtr<ID3D12Resource>		InsUploader;
	unsigned int				InsCount;
	std::vector<InstanceData>	InsData;

};

#endif