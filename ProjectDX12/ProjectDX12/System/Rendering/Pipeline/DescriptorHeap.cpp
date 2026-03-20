
#include "System/Rendering/Pipeline/DescriptorHeap.h"

/*
* ステージ用の非シェーダー可視ヒープを別途作る（D3D12_DESCRIPTOR_HEAP_FLAG_NONE）。CPU で作成／書き込みするヒープをソースにして、宛先をシェーダー可視ヒープにして CopyDescriptorsSimple を呼ぶ。
*/

DescriptorHeap::DescriptorHeap(Description desc)
	:
	AllocCout(0),
	Type(),
	visibleShader(false)
{
	// ディスクリプターヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC heap = {};
	Type = heap.Type = desc.heapType;

	// シェーダーから参照するか
	if (desc.staging)
	{
		heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	}
	else
	{
		if (heap.Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		{
			visibleShader = true;
			heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		}
	}
	heap.NumDescriptors	= desc.num;
	heap.NodeMask = 0;

	HRESULT hr = GetDevice()->CreateDescriptorHeap(&heap, IID_PPV_ARGS(Heap.GetAddressOf()));
}

DescriptorHeap::~DescriptorHeap()
{
}

DescriptorHeap::Handle DescriptorHeap::Allocate()
{
	Handle handle;

	// 目的のデスクリプタにアクセス
	UINT increment = GetDevice()->GetDescriptorHandleIncrementSize(Type);	
	increment *= AllocCout;

	D3D12_CPU_DESCRIPTOR_HANDLE hCPU = Heap->GetCPUDescriptorHandleForHeapStart();
	hCPU.ptr	+= increment;
	handle.hCPU = hCPU;

	if (visibleShader)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE hGPU = Heap->GetGPUDescriptorHandleForHeapStart();
		hGPU.ptr += increment;
		handle.hGPU = hGPU;
	}

	handle.Type = Type;

	// カウンタ更新
	++AllocCout;

	return handle;
}

void DescriptorHeap::Bind()
{
	GetCommandList()->SetDescriptorHeaps(1, &Heap);
}

void DescriptorHeap::Bind(ID3D12DescriptorHeap** _heaps, UINT _num)
{
	GetCommandList()->SetDescriptorHeaps(_num, _heaps);
}
