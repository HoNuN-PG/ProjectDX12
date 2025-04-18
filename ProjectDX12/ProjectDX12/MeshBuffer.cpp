
#include "MeshBuffer.h"

MeshBuffer::MeshBuffer(Description desc)
	: 
	m_desc(desc), 
	m_pVtxBuf(nullptr), 
	m_vbv{}, 
	m_pIdxBuf(nullptr), 
	m_ibv{}
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
		IID_PPV_ARGS(&m_pVtxBuf)
	);
	if (FAILED(hr)) { return; }

	// 頂点バッファの初期値設定
	void* pVtxMap;
	hr = m_pVtxBuf->Map(0, nullptr, (void**)&pVtxMap);
	if (SUCCEEDED(hr)) {
		memcpy_s(pVtxMap, resDesc.Width, desc.pVtx, resDesc.Width);
	}
	m_pVtxBuf->Unmap(0, nullptr);

	// 頂点バッファビューの設定
	m_vbv.BufferLocation	= m_pVtxBuf->GetGPUVirtualAddress();
	m_vbv.SizeInBytes		= resDesc.Width;
	m_vbv.StrideInBytes		= desc.vtxSize;

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
	hr = GetDevice()->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&m_pIdxBuf));

	// インデックスデータ書き込み
	void* pIdxMap = nullptr;
	hr = m_pIdxBuf->Map(0, nullptr, &pIdxMap);
	if (SUCCEEDED(hr)) {
		memcpy_s(pIdxMap, resDesc.Width, desc.pIdx, resDesc.Width);
	}
	m_pIdxBuf->Unmap(0, nullptr);

	// インデックスバッファビュー作成
	m_ibv.BufferLocation = m_pIdxBuf->GetGPUVirtualAddress();
	m_ibv.Format = desc.idxSize;
	m_ibv.SizeInBytes = static_cast<UINT>(resDesc.Width);
}

MeshBuffer::~MeshBuffer()
{
	if (m_pIdxBuf)
		m_pIdxBuf->Release();
	m_pVtxBuf->Release();
}

void MeshBuffer::Draw()
{
	GetCommandList()->IASetPrimitiveTopology(m_desc.topology);
	GetCommandList()->IASetVertexBuffers(0, 1, &m_vbv);
	if (m_pIdxBuf)
	{
		GetCommandList()->IASetIndexBuffer(&m_ibv);
		GetCommandList()->DrawIndexedInstanced(m_desc.idxCount, 1, 0, 0, 0);
	}
	else {
		GetCommandList()->DrawInstanced(m_desc.vtxCount, 1, 0, 0);
	}
}
