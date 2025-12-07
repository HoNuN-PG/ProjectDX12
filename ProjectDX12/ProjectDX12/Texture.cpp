
#include <DDSTextureLoader.h>
#include <ResourceUploadBatch.h>

#include <wrl/client.h>

// Rendering/Texture
#include "Texture.h"

Texture::Texture(Description desc)
{
	if (!strstr(desc.fileName, ".dds"))
	{ // DDS以外の読み込み
		DirectX::TexMetadata info;
		DirectX::ScratchImage image;
		HRESULT hr = LoadTexture(desc.fileName, &info, &image);
		if (FAILED(hr)) { return; }

		// リソース作成
		D3D12_HEAP_PROPERTIES texProp = {};
		texProp.Type = D3D12_HEAP_TYPE_CUSTOM;
		texProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		texProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Format = info.format;
		texDesc.Width = info.width;
		texDesc.Height = info.height;
		texDesc.DepthOrArraySize = info.arraySize;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.MipLevels = info.mipLevels;
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// リソースの作成
		hr = GetDevice()->CreateCommittedResource(&texProp, D3D12_HEAP_FLAG_NONE,
			&texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&Resource));
		if (FAILED(hr)) { return; }

		for (size_t mip = 0; mip < info.mipLevels; ++mip)
		{
			const DirectX::Image* img = image.GetImage(mip, 0, 0);
			Resource->WriteToSubresource((UINT)mip, nullptr, img->pixels, img->rowPitch, img->slicePitch);
		}

		// シェーダーリソースビューの作成
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = info.format;
		srvDesc.Texture2D.MipLevels = info.mipLevels;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		// ディスクリプターの割り当て位置(事前に定数バッファを作っているのでハンドルのポインタを移動させる)
		Handle = desc.pHeap->Allocate();
		GetDevice()->CreateShaderResourceView(Resource, &srvDesc, Handle.hCPU);
	}
	else
	{ // DDSの読み込み
		wchar_t wPath[MAX_PATH];
		size_t wLen = 0;
		MultiByteToWideChar(0, 0, desc.fileName, -1, wPath, MAX_PATH);

		// テクスチャロード
		DirectX::ResourceUploadBatch upload(GetDevice());
		upload.Begin();
		DirectX::CreateDDSTextureFromFile(
			GetDevice(), upload, wPath, &Resource
		);
		auto finish = upload.End(GetCommandQueue());
		finish.wait();

		// シェーダーリソースビューの作成
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = Resource->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = Resource->GetDesc().MipLevels;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		// ディスクリプターの割り当て位置(事前に定数バッファを作っているのでハンドルのポインタを移動させる)
		Handle = desc.pHeap->Allocate();
		GetDevice()->CreateShaderResourceView(Resource, &srvDesc, Handle.hCPU);
	}
}

Texture::~Texture()
{
	Resource->Release();
}
