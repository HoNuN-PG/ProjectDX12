#ifndef ___MESH_BUFFER_H___
#define ___MESH_BUFFER_H___

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
	MeshBuffer(Description desc);
	~MeshBuffer();
	void Draw();

private:
	Description					Desc;
	ID3D12Resource*				Vtx;
	D3D12_VERTEX_BUFFER_VIEW	Vbv;
	ID3D12Resource*				Idx;
	D3D12_INDEX_BUFFER_VIEW		Ibv;

};

#endif