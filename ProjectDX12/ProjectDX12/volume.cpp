
#include "volume.h"

#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

std::unique_ptr<MeshBuffer>					Volume::Screen;
std::shared_ptr<DescriptorHeap>				Volume::Heap;
std::shared_ptr<DescriptorHeap>				Volume::RTVHeap;
std::unique_ptr<RenderTarget>				Volume::PostProcessRTV;

void Volume::Load()
{
	// スクリーン頂点
	Vertex screenVtx[] =
	{
		{{-0.5f, 0.5f,0} ,{0,0}} ,
		{{ 0.5f, 0.5f,0} ,{1,0}} ,
		{{-0.5f,-0.5f,0} ,{0,1}} ,
		{{ 0.5f,-0.5f,0} ,{1,1}} ,
	};
	// スクリーン
	{
		MeshBuffer::Description desc = {};
		desc.pVtx = screenVtx;
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = _countof(screenVtx);
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		Screen = std::make_unique<MeshBuffer>(desc);
	}
	// ボリュームディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 128;
		Heap = std::make_shared<DescriptorHeap>(desc);
	}
	// ボリュームディスクリプターヒープ(レンダーターゲット)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = 64;
		RTVHeap = std::make_shared<DescriptorHeap>(desc);
	}
	// PostProcessRTV
	{
		RenderTarget::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.pRTVHeap = RTVHeap.get();
		desc.pSRVHeap = Heap.get();
		PostProcessRTV = std::make_unique<RenderTarget>(desc);
	}
}

void Volume::Unload()
{
}

void Volume::CopyTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE src, D3D12_CPU_DESCRIPTOR_HANDLE dest)
{
	GetDevice()->CopyDescriptorsSimple(
		1,
		dest,
		src,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Volume::CopyGlobalTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT key)
{
	GetDevice()->CopyDescriptorsSimple(
		1,
		dest,
		RenderingEngine::GetGlobalTextureSRV(key).hCPU,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Volume::BindPostProcessRTV()
{
	// RTVの設定
	constexpr static float clearColor[4] = { 0,0,0,0 };
	PostProcessRTV->SRV2RTV();
	PostProcessRTV->Clear(clearColor);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		PostProcessRTV->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs);
}

void Volume::BindPipeline(UINT idx)
{
	PipelineData[idx]->Bind();
}

void Volume::BindHeap()
{
	ID3D12DescriptorHeap* heaps[] =
	{
		Heap->Get(),
	};
	DescriptorHeap::Bind(heaps, 1);
}

void Volume::Rendering()
{
	// 表示領域の設定
	SetViewPort(WINDOW_WIDTH, WINDOW_HEIGHT);
	Screen->Draw();
}
