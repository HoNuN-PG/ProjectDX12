
#include "Copy.h"
#include "DescriptorHeap.h"
#include "RenderTarget.h"

std::unique_ptr<MeshBuffer>	Copy::Screen;
std::unique_ptr<RootSignature> Copy::RootSignatureData;
std::unique_ptr<Pipeline> Copy::PipelineData;

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
	Screen = std::make_unique<MeshBuffer>(desc);

	// ルートシグネチャ
	{
		RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::DescriptionTable desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		RootSignatureData = std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"assets/shader/VS_Sprite.cso";
		desc.PSFile = L"assets/shader/PS_Copy.cso";
		desc.pRootSignature = RootSignatureData->Get();
		desc.RenderTargetNum = 1;
		PipelineData = std::make_unique<Pipeline>(desc);
	}
}

void Copy::ExecuteCopy(DescriptorHeap* heap, RenderTarget* src, D3D12_CPU_DESCRIPTOR_HANDLE dst)
{
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	// 表示領域の設定
	D3D12_VIEWPORT vp = { 0, 0, 1280.0f, 720.0f, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, 1280.0f, 720.0f };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	// レンダーターゲット切り替え
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		dst,
	};
	SetRenderTarget(1, rtvs);

	// ImGUI用RTVに描画
	PipelineData->Bind();
	ID3D12DescriptorHeap* heaps[] =
	{
		heap->Get(),
	};
	DescriptorHeap::Bind(heaps, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen[] = {
		src->GetHandleSRV().hGPU,
	};
	RootSignatureData->Bind(hScreen, _countof(hScreen));
	Screen->Draw();
}
