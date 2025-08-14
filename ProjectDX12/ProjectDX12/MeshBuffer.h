#ifndef ___MESH_BUFFER_H___
#define ___MESH_BUFFER_H___

#include "DirectX.h"
#include <DirectXMath.h>
#include <vector>

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
	static const int MAX_INSTANCE = 100;
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
	ID3D12Resource*				Vtx;
	D3D12_VERTEX_BUFFER_VIEW	Vbv;
	ID3D12Resource*				Idx;
	D3D12_INDEX_BUFFER_VIEW		Ibv;

};

class InstanceMeshBuffer : MeshBuffer
{
public:
	InstanceMeshBuffer() {};
	InstanceMeshBuffer(Description desc,unsigned int count = 0);
	virtual ~InstanceMeshBuffer() {};
public:
	void MappingUploder();
	virtual void Draw() override;
private:
	ID3D12Resource*				Ins;
	ID3D12Resource*				InsUploader;
	unsigned int				InsCount;
	std::vector<InstanceData>	InsData;

};

#endif