#ifndef ___DESCRIPTOR_HEAP_H___
#define ___DESCRIPTOR_HEAP_H___

// System
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
	struct Handle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE hCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE hGPU;
	};

public:
	DescriptorHeap(Description desc);
	~DescriptorHeap();
	ID3D12DescriptorHeap* Get() { return Heap; }
	// ディスクリプタを取得
	Handle Allocate();
	// ヒープの設定
	void Bind();
	static void Bind(ID3D12DescriptorHeap** _heaps, UINT _num);

private:
	ID3D12DescriptorHeap*		Heap;		// ヒープのデータ
	D3D12_DESCRIPTOR_HEAP_TYPE	Type;		// ディスクリプタヒープの種類
	UINT						AllocCout;	// 確保したディスクリプタ数

};

#endif
