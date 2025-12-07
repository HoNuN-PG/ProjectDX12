#ifndef ___RENDER_TARGET_H___
#define ___RENDER_TARGET_H___

// System/Rendering/Pipeline
#include "DescriptorHeap.h"

class RenderTarget
{
public:
	struct Description
	{
		UINT width;
		UINT height;
		DXGI_FORMAT format;
		DescriptorHeap* pRTVHeap;
		DescriptorHeap* pSRVHeap;
	};

public:
	RenderTarget() = delete;
	RenderTarget(Description desc);
	~RenderTarget();
	void RTV2SRV();
	void SRV2RTV();
	void ResourceBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	static void ResourceBarrier(ID3D12Resource* res, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	void Clear();
	void Clear(const float clearColor[]);
	ID3D12Resource* GetResource() { return Resource; }
	DescriptorHeap::Handle GetHandleRTV() { return hRTV; }
	DescriptorHeap::Handle GetHandleSRV() { return hSRV; }

public:
	float Width;
	float Height;

private:
	ID3D12Resource*			Resource;
	DescriptorHeap::Handle	hRTV;
	DescriptorHeap::Handle	hSRV;

};

#endif
