
// System/Constant
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(Description desc)
	:
	Handle{},
	Size(0),
	Resource(nullptr),
	CBV{},
	Ptr(nullptr)
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
	res.Width					= (desc.size + 0xff) & ~0xff; // 256バイトアライメント
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
		nullptr, IID_PPV_ARGS(&Resource));
	if (FAILED(hr)) { return; }

	// 定数バッファビュー生成
	CBV.BufferLocation	= Resource->GetGPUVirtualAddress();
	CBV.SizeInBytes		= static_cast<UINT>(res.Width);		// 256アライメントでないとGPUアクセスエラー
	Handle				= desc.pHeap->Allocate();
	pDevice->CreateConstantBufferView(&CBV, Handle.hCPU);	// ディスクリプターヒープとの紐づけ

	// データ初期化
	hr = Resource->Map(0, nullptr, &Ptr);
	if (FAILED(hr)) { return; }
	ZeroMemory(Ptr, desc.size);
	Size = desc.size;
}

ConstantBuffer::~ConstantBuffer()
{
	if(Resource) Resource->Release();
}
