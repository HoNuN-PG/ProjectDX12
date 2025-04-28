
#include "DescriptorHeap.h"

DescriptorHeap::DescriptorHeap(Description desc)
	:
	m_allocCout(0),
	m_type()
{
	// ディスクリプターヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC heap = {};
	m_type = heap.Type		= desc.heapType;
	// シェーダーから参照するか
	if (desc.heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heap.NumDescriptors		= desc.num;
	heap.NodeMask			= 0;

	HRESULT hr = GetDevice()->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&m_pHeap));
}

DescriptorHeap::~DescriptorHeap()
{
	m_pHeap->Release();
}

DescriptorHeap::Handle DescriptorHeap::Allocate()
{
	D3D12_CPU_DESCRIPTOR_HANDLE hCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE hGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();
	// GPUごとのデータサイズを取得
	UINT increment = GetDevice()->GetDescriptorHandleIncrementSize(m_type);
	// 目的のデスクリプタにアクセス
	increment	*= m_allocCout;
	hCPU.ptr	+= increment;
	hGPU.ptr	+= increment;
	++m_allocCout;

	Handle handle;
	handle.hCPU = hCPU;
	handle.hGPU = hGPU;
	return handle;
}

void DescriptorHeap::Bind()
{
	GetCommandList()->SetDescriptorHeaps(1, &m_pHeap);
}

void DescriptorHeap::Bind(ID3D12DescriptorHeap* _heaps, UINT _num)
{
	GetCommandList()->SetDescriptorHeaps(_num, &_heaps);
}
