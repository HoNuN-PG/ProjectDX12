
#include "Texture.h"

#include <DirectXTex.h>
#include <TextureLoad.h>

cTexture::cTexture(sDescription desc)
{
	DirectX::TexMetadata info;
	DirectX::ScratchImage image;
	HRESULT hr = LoadTexture(desc.fileName, &info, &image);
	if (FAILED(hr)) { return; }

	// リソース作成
	D3D12_HEAP_PROPERTIES texProp	= {};
	texProp.Type					= D3D12_HEAP_TYPE_CUSTOM;
	texProp.CPUPageProperty			= D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texProp.MemoryPoolPreference	= D3D12_MEMORY_POOL_L0;

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Format				= info.format;
	texDesc.Width				= info.width;
	texDesc.Height				= info.height;
	texDesc.DepthOrArraySize	= info.arraySize;
	texDesc.SampleDesc.Count	= 1;
	texDesc.SampleDesc.Quality	= 0;
	texDesc.MipLevels			= info.mipLevels;
	texDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags				= D3D12_RESOURCE_FLAG_NONE;

	// リソースの作成
	hr = GetDevice()->CreateCommittedResource(&texProp, D3D12_HEAP_FLAG_NONE,
		&texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_pTexture));
	if (FAILED(hr)) { return; }

	// 書き込み
	auto img = image.GetImage(0, 0, 0);
	hr = m_pTexture->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch);
	if (FAILED(hr)) { return; }

	// シェーダーリソースビューの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension					= D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format							= info.format;
	srvDesc.Shader4ComponentMapping			= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels				= info.mipLevels;

	// ディスクリプターの割り当て位置(事前に定数バッファを作っているのでハンドルのポインタを移動させる)
	m_handle = desc.pHeap->Allocate();
	GetDevice()->CreateShaderResourceView(m_pTexture, &srvDesc, m_handle.hCPU);
}

cTexture::~cTexture()
{
	m_pTexture->Release();
}
