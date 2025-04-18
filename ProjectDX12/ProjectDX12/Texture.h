#ifndef ___TEXTURE_H___
#define ___TEXTURE_H___

#include "DescriptorHeap.h"

class cTexture
{
public:
	struct sDescription
	{
		const char*			fileName;
		DescriptorHeap*	pHeap;
	};
public:
	cTexture(sDescription desc);
	~cTexture();
	DescriptorHeap::sHandle GetHandle() {
		return m_handle;
	}
private:
	ID3D12Resource*				m_pTexture;	// テクスチャリソース
	DescriptorHeap::sHandle	m_handle;
};

#endif
