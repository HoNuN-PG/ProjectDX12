#ifndef ___TEXTURE_H___
#define ___TEXTURE_H___

#include <TextureLoad.h>

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
	DescriptorHeap::Handle GetHandle() 
	{ return Handle; }

private:
	ID3D12Resource*			Resource;	// テクスチャリソース
	ID3D12Resource*			Uploader;	// アップローダ
	DescriptorHeap::Handle	Handle;

};

#endif
