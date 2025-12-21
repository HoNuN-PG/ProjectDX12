
// Model
#include "MeshBuffer.h"

MeshBuffer::MeshBuffer(Description desc)
	: 
	Desc(desc), 
	Vtx(nullptr), 
	Vbv{}, 
	Idx(nullptr), 
	Ibv{}
{
	HRESULT hr;
	// ヒープの設定
	D3D12_HEAP_PROPERTIES heapProp	= {};
	heapProp.Type						= D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty			= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference		= D3D12_MEMORY_POOL_UNKNOWN;
	heapProp.CreationNodeMask			= 1;
	heapProp.VisibleNodeMask			= 1;

	// 頂点バッファリソースの設定
	D3D12_RESOURCE_DESC resDesc	= {};
	resDesc.Dimension				= D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width					= desc.vtxSize * desc.vtxCount;
	resDesc.Height					= 1;
	resDesc.DepthOrArraySize		= 1;
	resDesc.MipLevels				= 1;
	resDesc.Format					= DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count		= 1;
	resDesc.Layout					= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags					= D3D12_RESOURCE_FLAG_NONE;

	// 頂点バッファリソースの生成
	hr = GetDevice()->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&Vtx)
	);
	if (FAILED(hr)) { return; }

	// 頂点バッファの初期値設定
	void* pVtxMap;
	hr = Vtx->Map(0, nullptr, (void**)&pVtxMap);
	if (SUCCEEDED(hr)) 
	{
		memcpy_s(pVtxMap, resDesc.Width, desc.pVtx, resDesc.Width);
	}
	Vtx->Unmap(0, nullptr);

	// 頂点バッファビューの設定
	Vbv.BufferLocation	= Vtx->GetGPUVirtualAddress();
	Vbv.SizeInBytes		= resDesc.Width;
	Vbv.StrideInBytes	= desc.vtxSize;

	// インデックス作成チェック
	if (desc.pIdx == nullptr) { return; }

	// リソース設定
	UINT idxSize = 0;
	switch (desc.idxSize)
	{
	case DXGI_FORMAT_R8_UINT: idxSize = 1; break;
	case DXGI_FORMAT_R16_UINT:idxSize = 2; break;
	case DXGI_FORMAT_R32_UINT:idxSize = 4; break;
	}
	resDesc.Width = idxSize * desc.idxCount;

	// リソース生成
	hr = GetDevice()->CreateCommittedResource(
		&heapProp, 
		D3D12_HEAP_FLAG_NONE, 
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Idx)
	);

	// インデックスデータ書き込み
	void* pIdxMap	= nullptr;
	hr				= Idx->Map(0, nullptr, &pIdxMap);
	if (SUCCEEDED(hr)) 
	{
		memcpy_s(pIdxMap, resDesc.Width, desc.pIdx, resDesc.Width);
	}
	Idx->Unmap(0, nullptr);

	// インデックスバッファビュー作成
	Ibv.BufferLocation	= Idx->GetGPUVirtualAddress();
	Ibv.Format			= desc.idxSize;
	Ibv.SizeInBytes		= static_cast<UINT>(resDesc.Width);
}

MeshBuffer::~MeshBuffer()
{
}

void MeshBuffer::Draw()
{
	GetCommandList()->IASetPrimitiveTopology(Desc.topology);
	GetCommandList()->IASetVertexBuffers(0, 1, &Vbv);
	if (Idx)
	{
		GetCommandList()->IASetIndexBuffer(&Ibv);
		GetCommandList()->DrawIndexedInstanced(Desc.idxCount, 1, 0, 0, 0);
	}
	else 
	{
		GetCommandList()->DrawInstanced(Desc.vtxCount, 1, 0, 0);
	}
}

InstanceMeshBuffer::InstanceMeshBuffer(Description desc, unsigned int count) 
	:
	MeshBuffer(desc)
{
	if(count == 1)
	{
		InsCount = 1;
		bInstanced = false;
		return;
	}

	InsCount = count;
	bInstanced = true;

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type					= D3D12_HEAP_TYPE_DEFAULT;
	heapProp.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
	heapProp.CreationNodeMask		= 1;
	heapProp.VisibleNodeMask		= 1;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width				= sizeof(InstanceData) * max(InsCount, MAX_INSTANCE);
	resDesc.Height				= 1;
	resDesc.DepthOrArraySize	= 1;
	resDesc.MipLevels			= 1;
	resDesc.Format				= DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count	= 1;
	resDesc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags				= D3D12_RESOURCE_FLAG_NONE;

	auto result = GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&Ins)
	);
	if (FAILED(result)) { return; }

	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	result = GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&InsUploader)
	);
	if (FAILED(result)) { return; }
}

InstanceMeshBuffer::~InstanceMeshBuffer()
{
}

void InstanceMeshBuffer::MappingUploder()
{
	InstanceData* mappedData;
	const size_t dataSize = sizeof(InstanceData);

	auto result = InsUploader->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
	if (FAILED(result)) { return; }

	memcpy(mappedData, InsData.data(), dataSize * InsData.size());

	InsUploader->Unmap(0, nullptr);

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData		= mappedData;
	subResourceData.RowPitch	= dataSize;
	subResourceData.SlicePitch	= subResourceData.RowPitch;

	UpdateSubresources(
		GetCommandList(), 
		Ins.Get(), 
		InsUploader.Get(), 
		0, 
		0, 
		1, 
		&subResourceData
	);
}

void InstanceMeshBuffer::Draw()
{
	GetCommandList()->IASetPrimitiveTopology(Desc.topology);
	GetCommandList()->IASetVertexBuffers(0, 1, &Vbv);
	if (Idx)
	{
		GetCommandList()->IASetIndexBuffer(&Ibv);
		GetCommandList()->DrawIndexedInstanced(Desc.idxCount, InsCount, 0, 0, 0);
	}
	else 
	{
		GetCommandList()->DrawInstanced(Desc.vtxCount, 1, 0, 0);
	}
}
