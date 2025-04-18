
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(Description desc)
	:
	m_handle{},
	m_size(0),
	m_pBuf(nullptr),
	m_cbv{},
	m_pPtr(nullptr)
{
	HRESULT hr;
	ID3D12Device* pDevice = GetDevice();

	// ヒーププロパティ設定
	D3D12_HEAP_PROPERTIES prop	= {};
	prop.Type					= D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask		= 1;
	prop.VisibleNodeMask		= 1;

	// リソースの設定
	D3D12_RESOURCE_DESC res		= {};
	res.Dimension				= D3D12_RESOURCE_DIMENSION_BUFFER;
	res.Alignment				= 0;
	res.Width					= (desc.size + 0xff) & ~0xff; // 定数バッファのサイズ(256バイトアライメント
	res.Height					= 1;
	res.DepthOrArraySize		= 1;
	res.MipLevels				= 1;
	res.Format					= DXGI_FORMAT_UNKNOWN;
	res.SampleDesc.Count		= 1;
	res.SampleDesc.Quality		= 0;
	res.Layout					= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	res.Flags					= D3D12_RESOURCE_FLAG_NONE;

	// リソース生成
	hr = pDevice->CreateCommittedResource(
		&prop, D3D12_HEAP_FLAG_NONE, &res, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&m_pBuf));
	if (FAILED(hr)) { return; }

	// 定数バッファビュー生成
	m_cbv.BufferLocation	= m_pBuf->GetGPUVirtualAddress();
	m_cbv.SizeInBytes		= static_cast<UINT>(res.Width);		// 256アライメントでないとGPUアクセスエラー
	m_handle				= desc.pHeap->Allocate();
	pDevice->CreateConstantBufferView(&m_cbv, m_handle.hCPU);	// ディスクリプターヒープとの紐づけ

	// データ初期化
	hr = m_pBuf->Map(0, nullptr, &m_pPtr);
	if (FAILED(hr)) { return; }
	ZeroMemory(m_pPtr, desc.size);
	m_size = desc.size;
}

ConstantBuffer::~ConstantBuffer()
{
	m_pBuf->Release();
}
