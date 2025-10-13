#ifndef ___TEXTURE_H___
#define ___TEXTURE_H___

#include <DirectXTex.h>
#include <TextureLoad.h>
#pragma comment(lib, "DirectXTex.lib")

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
	DescriptorHeap::Handle	Handle;

};

#endif
