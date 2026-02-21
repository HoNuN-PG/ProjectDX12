#ifndef ___DESCRIPTOR_HEAP_H___
#define ___DESCRIPTOR_HEAP_H___

#include "System/DirectX.h"

class DescriptorHeap
{
public:

	struct Description
	{
		D3D12_DESCRIPTOR_HEAP_TYPE	heapType;	// ヒープの種類
		UINT						num;		// ディスクリプタの最大数
	};

	struct Handle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE hCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE hGPU;
	};

public:

	DescriptorHeap(Description desc);
	~DescriptorHeap();

public:

	/// <summary>
	/// 使用可能なディスクリプタの取得
	/// </summary>
	/// <returns></returns>
	Handle Allocate();

public:

	/// <summary>
	/// コマンドリストにヒープを設定
	/// </summary>
	void Bind();
	static void Bind(ID3D12DescriptorHeap** _heaps, UINT _num);

public:

	ID3D12DescriptorHeap* Get() { return Heap.Get(); }

private:

	ComPtr<ID3D12DescriptorHeap>		Heap;		// ヒープ
	D3D12_DESCRIPTOR_HEAP_TYPE			Type;		// ディスクリプタヒープの種類
	UINT								AllocCout;	// 確保したディスクリプタ数

};

#endif
