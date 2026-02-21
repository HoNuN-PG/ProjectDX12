#ifndef ___TEXTURE_H___
#define ___TEXTURE_H___

#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "TextureLoad.h"

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

public:

	DescriptorHeap::Handle GetHandle() { return Handle; }

private:

	ComPtr<ID3D12Resource>	Resource;	// テクスチャリソース
	DescriptorHeap::Handle	Handle;

};

#endif
