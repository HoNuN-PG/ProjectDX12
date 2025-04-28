#ifndef ___RENDER_TARGET_H___
#define ___RENDER_TARGET_H___

#include "DescriptorHeap.h"

class RenderTarget
{
public:
	struct Description
	{
		UINT width;
		UINT height;
		DescriptorHeap* pRTVHeap;
		DescriptorHeap* pSRVHeap;
	};

public:
	RenderTarget(Description desc);
	~RenderTarget();
	void ResourceBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	void Clear();
	void Clear(const float clearColor[]);
	DescriptorHeap::Handle GetHandleRTV() {
		return m_hRTV;
	}
	DescriptorHeap::Handle GetHandleSRV() {
		return m_hSRV;
	}

private:
	ID3D12Resource*			m_pRenderTarget;
	DescriptorHeap::Handle	m_hRTV;
	DescriptorHeap::Handle	m_hSRV;

};

#endif
