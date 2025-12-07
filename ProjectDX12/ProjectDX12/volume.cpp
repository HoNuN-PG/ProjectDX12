
// System/Rendering/Volume
#include "volume.h"
// System/Rendering
#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

std::unique_ptr<MeshBuffer>	Volume::Screen;

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
}

void Volume::Unload()
{
	if (Screen)
	{
		Screen.reset(nullptr);
	}
}

void Volume::Init(UINT heapNum, UINT rtvNum)
{
	// ボリュームディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = heapNum + 1;
		Heap = std::make_shared<DescriptorHeap>(desc);
	}
	// ボリュームディスクリプターヒープ(レンダーターゲット)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = rtvNum + 1;
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

void Volume::BindHeap()
{
	ID3D12DescriptorHeap* heaps[] =
	{
		Heap->Get(),
	};
	DescriptorHeap::Bind(heaps, 1);
}

void Volume::BindPipeline(UINT idx)
{
	PipelineData[idx]->Bind();
}

void Volume::BindPostProcessRTV()
{
	// RTVの設定
	constexpr static float clearColor[4] = { 0,0,0,0 };
	PostProcessRTV->SRV2RTV();
	PostProcessRTV->Clear(clearColor);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		PostProcessRTV->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs);
}

void Volume::Rendering()
{
	// 表示領域の設定
	SetViewPort(WINDOW_WIDTH, WINDOW_HEIGHT);
	Screen->Draw();
}
