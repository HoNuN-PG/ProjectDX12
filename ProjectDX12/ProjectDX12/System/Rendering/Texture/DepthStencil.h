#ifndef ___DEPTH_STENCIL_H___
#define ___DEPTH_STENCIL_H___

#include "System/Rendering/Pipeline/DescriptorHeap.h"

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
	~DepthStencil();

public:

	void Clear();

public:

	DescriptorHeap::Handle GetHandleDSV();

private:

	ComPtr<ID3D12Resource> Resource;
	DescriptorHeap::Handle DSV;

};

#endif