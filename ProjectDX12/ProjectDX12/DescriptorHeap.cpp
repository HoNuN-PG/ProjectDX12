
// System/Pipeline
#include "DescriptorHeap.h"

DescriptorHeap::DescriptorHeap(Description desc)
	:
	AllocCout(0),
	Type()
{
	// ディスクリプターヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC heap = {};
	Type = heap.Type				= desc.heapType;

	// シェーダーから参照するか
	if (desc.heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heap.NumDescriptors		= desc.num;
	heap.NodeMask			= 0;

	HRESULT hr = GetDevice()->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&Heap));
}

DescriptorHeap::~DescriptorHeap()
{
	if(Heap) Heap->Release();
}

DescriptorHeap::Handle DescriptorHeap::Allocate()
{
	D3D12_CPU_DESCRIPTOR_HANDLE hCPU = Heap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE hGPU = Heap->GetGPUDescriptorHandleForHeapStart();
	// GPUごとのデータサイズを取得
	UINT increment = GetDevice()->GetDescriptorHandleIncrementSize(Type);
	// 目的のデスクリプタにアクセス
	increment	*= AllocCout;
	hCPU.ptr	+= increment;
	hGPU.ptr	+= increment;
	++AllocCout;

	Handle handle;
	handle.hCPU = hCPU;
	handle.hGPU = hGPU;
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
