
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Texture/RenderTarget.h"
#include "System/Rendering/Volume/Copy.h"
#include "System/DirectX.h"

Copy::Copy(MeshBuffer& screen)
	: pScreen(&screen)
{
	Load();
}

void Copy::Load()
{
	// ルートシグネチャ
	{
		RootSignature::Parameter param[] = 
		{
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description rootsignature =
		{
			param,
			_countof(param),
			D3D12_FILTER_MIN_MAG_MIP_LINEAR
		};
		pRootSignatureData = std::make_unique<RootSignature>(rootsignature);
	}
	// パイプライン
	{
		PipelineState::Description desc = {
			L"",
			L"",
			L"../game/assets/shader/VS_Sprite.cso",
			L"../game/assets/shader/PS_Copy.cso",
			pRootSignatureData->Get(),
			PipelineState::IED_POS_TEX,
			PipelineState::IED_POS_TEX_COUNT,
			D3D12_CULL_MODE_BACK,
			std::vector<DXGI_FORMAT>(),
			FALSE,
		};

		desc.RenderTargetFormat.clear();
		desc.RenderTargetFormat.push_back(DXGI_FORMAT_R16G16B16A16_FLOAT);
		pPipelineData[DXGI_FORMAT_R16G16B16A16_FLOAT] = std::make_unique<PipelineState>(desc);

		desc.RenderTargetFormat.clear();
		desc.RenderTargetFormat.push_back(DXGI_FORMAT_R16G16_FLOAT);
		pPipelineData[DXGI_FORMAT_R16G16_FLOAT] = std::make_unique<PipelineState>(desc);

		desc.RenderTargetFormat.clear();
		desc.RenderTargetFormat.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
		pPipelineData[DXGI_FORMAT_R8G8B8A8_UNORM] = std::make_unique<PipelineState>(desc);
	}
}

void Copy::ExecuteCopy2BBuffer(DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE src)
{
	// 表示領域の設定
	SetViewPort(WINDOW_WIDTH,WINDOW_HEIGHT);

	// レンダーターゲット切り替え
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		GetBBuffer(),
	};
	SetRenderTarget(1, rtvs);

	ID3D12DescriptorHeap* heaps[] =
	{
		heap->Get(),
	};
	DescriptorHeap::Bind(heaps, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen[] = 
	{
		src,
	};
	pRootSignatureData->Bind(hScreen, _countof(hScreen));
	pPipelineData[DXGI_FORMAT_R8G8B8A8_UNORM]->Bind();
	pScreen->Draw();
}

void Copy::ExecuteCopy(DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE src, RenderTarget* dest)
{
	// 表示領域の設定
	SetViewPort(dest->Width,dest->Height);

	std::vector<RenderTarget*> targets = { dest };
	D3D12_GPU_DESCRIPTOR_HANDLE handle[] =
	{
		src,
	};
	ScreenDrawNoClear(
		targets,
		heap->Get(),
		handle,
		_countof(handle),
		pRootSignatureData.get(),
		pPipelineData[dest->Format].get(),
		pScreen
	);

	SetViewPort(WINDOW_WIDTH,WINDOW_HEIGHT);
}
