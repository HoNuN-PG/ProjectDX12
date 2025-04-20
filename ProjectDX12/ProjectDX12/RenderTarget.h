#ifndef ___RENDER_TARGET_H___
#define ___RENDER_TARGET_H___

#include "DescriptorHeap.h"

class RenderTarget
{
public:
	struct sDescription
	{
		UINT width;
		UINT height;
		DescriptorHeap* pRTVHeap;
		DescriptorHeap* pSRVHeap;
	};
public:
	RenderTarget(sDescription desc);
	~RenderTarget();
	void ResourceBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	void Clear();
	void Clear(const float clearColor[]);
	DescriptorHeap::sHandle GetHandleRTV() {
		return m_hRTV;
	}
	DescriptorHeap::sHandle GetHandleSRV() {
		return m_hSRV;
	}
private:
	ID3D12Resource*				m_pRenderTarget;
	DescriptorHeap::sHandle	m_hRTV;
	DescriptorHeap::sHandle	m_hSRV;
};

#endif
