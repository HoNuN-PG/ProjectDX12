
// System/Rendering/Pipeline
#include "DescriptorHeap.h"
// System/Rendering/Texture
#include "RenderTarget.h"
// System/Rendering/Volume
#include "Copy.h"

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
	Instance->Screen = std::make_unique<MeshBuffer>(desc);

	// ルートシグネチャ
	{
		RootSignature::ParameterTable param[] = 
		{
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::DescriptionTable desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		desc.filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		Instance->RootSignatureData = std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		Pipeline::InputLayout layout[] = 
		{
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.cull = D3D12_CULL_MODE_BACK;
		desc.VSFile = L"../exe/assets/shader/VS_Sprite.cso";
		desc.PSFile = L"../exe/assets/shader/PS_Copy.cso";
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.pRootSignature = Instance->RootSignatureData->Get();
		desc.RenderTargetNum = 1;
		Instance->PipelineData = std::make_unique<Pipeline>(desc);
	}
}

void Copy::ExecuteScreenDraw()
{
	Instance->Screen->Draw();
}

void Copy::ExecuteCopy(DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE src, D3D12_CPU_DESCRIPTOR_HANDLE dest)
{
	// 表示領域の設定
	SetViewPort(WINDOW_WIDTH,WINDOW_HEIGHT);

	// レンダーターゲット切り替え
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		dest,
	};
	SetRenderTarget(1, rtvs);

	Instance->PipelineData->Bind();
	ID3D12DescriptorHeap* heaps[] =
	{
		heap->Get(),
	};
	DescriptorHeap::Bind(heaps, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen[] = 
	{
		src,
	};
	Instance->RootSignatureData->Bind(hScreen, _countof(hScreen));
	Instance->Screen->Draw();
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

	Instance->PipelineData->Bind();
	ID3D12DescriptorHeap* heaps[] =
	{
		heap->Get(),
	};
	DescriptorHeap::Bind(heaps, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen[] = 
	{
		src,
	};
	Instance->RootSignatureData->Bind(hScreen, _countof(hScreen));
	Instance->Screen->Draw();

	SetViewPort(WINDOW_WIDTH,WINDOW_HEIGHT);
}
