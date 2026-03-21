
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Texture/RenderTarget.h"
#include "System/Rendering/Volume/Copy.h"

std::unique_ptr<Copy> Copy::Instance;

void Copy::Create()
{
	if (!Instance)
	{
		Instance = std::make_unique<Copy>();
		Instance->Load();
	}
}

void Copy::Destroy()
{
	if (Instance)
	{
		Instance.reset(nullptr);
	}
}

void Copy::Load()
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
	MeshBuffer::Description desc = {};
	desc.pVtx = screenVtx;
	desc.vtxSize = sizeof(Vertex);
	desc.vtxCount = _countof(screenVtx);
	desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	Instance->pScreen = std::make_unique<MeshBuffer>(desc);

	// ルートシグネチャ
	{
		RootSignature::Parameter param[] = 
		{
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		desc.filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		Instance->pRootSignatureData = std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		PipelineState::Description desc = {};
		desc.pRootSignature = Instance->pRootSignatureData->Get();
		desc.VSFile = L"../game/assets/shader/VS_Sprite.cso";
		desc.PSFile = L"../game/assets/shader/PS_Copy.cso";
		desc.pInputLayout = PipelineState::IED_POS_TEX;
		desc.InputLayoutNum = PipelineState::IED_POS_TEX_COUNT;
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.RenderTargetNum = 1;
		desc.WriteDepth = FALSE;

		desc.RenderTargetFormat.push_back(DXGI_FORMAT_R16G16B16A16_FLOAT);
		Instance->pPipelineData[DXGI_FORMAT_R16G16B16A16_FLOAT] = std::make_unique<PipelineState>(desc);

		desc.RenderTargetFormat.clear();
		desc.RenderTargetFormat.push_back(DXGI_FORMAT_R16G16_FLOAT);
		Instance->pPipelineData[DXGI_FORMAT_R16G16_FLOAT] = std::make_unique<PipelineState>(desc);

		desc.RenderTargetFormat.clear();
		desc.RenderTargetFormat.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
		Instance->pPipelineData[DXGI_FORMAT_R8G8B8A8_UNORM] = std::make_unique<PipelineState>(desc);
	}
}

void Copy::ExecuteScreenDraw()
{
	Instance->pScreen->Draw();
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
	Instance->pRootSignatureData->Bind(hScreen, _countof(hScreen));
	Instance->pPipelineData[DXGI_FORMAT_R8G8B8A8_UNORM]->Bind();
	Instance->pScreen->Draw();
}

void Copy::ExecuteCopy(DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE src, std::shared_ptr<RenderTarget> dest)
{
	// 表示領域の設定
	SetViewPort(dest->Width,dest->Height);

	// レンダーターゲット切り替え
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		dest->GetHandleRTV().hCPU,
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
	Instance->pRootSignatureData->Bind(hScreen, _countof(hScreen));
	Instance->pPipelineData[dest->Format]->Bind();
	Instance->pScreen->Draw();

	SetViewPort(WINDOW_WIDTH,WINDOW_HEIGHT);
}
