#ifndef ___TEXTURE_H___
#define ___TEXTURE_H___

#include "DescriptorHeap.h"

class Texture
{
public:
	struct Description
	{
		const char*		fileName;
		DescriptorHeap*	pHeap;
	};

public:
	Texture(Description desc);
	~Texture();
	DescriptorHeap::Handle GetHandle() {
		return m_handle;
	}

private:
	ID3D12Resource*			m_pTexture;	// テクスチャリソース
	DescriptorHeap::Handle	m_handle;

};

#endif
