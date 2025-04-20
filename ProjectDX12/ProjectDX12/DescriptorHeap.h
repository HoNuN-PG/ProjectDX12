#ifndef ___DESCRIPTOR_HEAP_H___
#define ___DESCRIPTOR_HEAP_H___

#include "DirectX.h"

class DescriptorHeap
{
public:
	// 設定情報
	struct Description
	{
		D3D12_DESCRIPTOR_HEAP_TYPE	heapType;	// ヒープの種類
		UINT						num;		// ディスクリプタの最大数
	};
	// ハンドル情報
	struct sHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE hCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE hGPU;
	};
public:
	DescriptorHeap(Description desc);
	~DescriptorHeap();
	ID3D12DescriptorHeap* Get() { return m_pHeap; }
	// ディスクリプタのハンドルを取得
	sHandle Allocate();
	void Bind();
private:
	ID3D12DescriptorHeap*		m_pHeap;		// ヒープのデータ
	D3D12_DESCRIPTOR_HEAP_TYPE	m_type;			// ディスクリプタヒープの種類
	UINT						m_allocCout;	// 確保したディスクリプタ数
};

#endif
