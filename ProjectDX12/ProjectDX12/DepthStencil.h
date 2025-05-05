#ifndef ___DEPTH_STENCIL_H___
#define ___DEPTH_STENCIL_H___

#include "DescriptorHeap.h"

class DepthStencil 
{
public:
	struct Description 
	{
		UINT width;
		UINT height;
		DescriptorHeap* pDSVHeap;
	};

public:
	DepthStencil(Description desc);
	~DepthStencil() {}
	void Clear();
	DescriptorHeap::Handle GetHandleDSV();

private:
	ID3D12Resource* Resource;
	DescriptorHeap::Handle DSV;

};

#endif