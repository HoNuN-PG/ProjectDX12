
#include "DebugImGUI.h"

#include <tchar.h>

#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "RenderTarget.h"

#include "ConstantWVP.h"

DebugImGUI::DebugImGUI()
{
	// スクリーン頂点
	Vertex screenVtx[] =
	{
		{{-0.5f, 0.5f,0} ,{0,0},{1,1,1,1}} ,
		{{ 0.5f, 0.5f,0} ,{1,0},{1,1,1,1}} ,
		{{-0.5f,-0.5f,0} ,{0,1},{1,1,1,1}} ,
		{{ 0.5f,-0.5f,0} ,{1,1},{1,1,1,1}} ,
	};

	// スクリーン
	MeshBuffer::Description desc = {};
	desc.pVtx = screenVtx;
	desc.vtxSize = sizeof(Vertex);
	desc.vtxCount = _countof(screenVtx);
	desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	mpScreen = std::make_unique<MeshBuffer>(desc);
}

DebugImGUI::~DebugImGUI()
{
}

MSG DebugImGUI::Create(HWND _hwnd)
{
	MSG msg = {};

	// ImGUI用のディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = HEAP_NUM;
		mpImGUIHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ImGUIの初期化
	// 実行ファイルの設定から高DPI項目の編集をする事でマウスポインタのズレ等を修正できる
	{
		if (ImGui::CreateContext() == nullptr) {
			MessageBox(_hwnd, _T("Error"), _T("Failed [Imgui]."), MB_OK);
			msg.message = WM_QUIT;
		}
		else {
			// Windows用のimguiの初期化
			if (!ImGui_ImplWin32_Init(_hwnd)) {
				MessageBox(_hwnd, _T("Error"), _T("Failed [Imgui]."), MB_OK);
				msg.message = WM_QUIT;
			}
			// DirectX用のimguiの初期化
			else {
				auto handle = mpImGUIHeap->Allocate();
				ImGui_ImplDX12_Init(GetDevice(), 3,
					DXGI_FORMAT_R8G8B8A8_UNORM, mpImGUIHeap->Get(), handle.hCPU, handle.hGPU);
			}
		}
	}
	// ルートシグネチャ
	{
		RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::DescriptionTable desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		mpRootSignature = std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		Pipeline::InputLayout layout[] = {
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"assets/shader/VS_Sprite.cso";
		desc.PSFile = L"assets/shader/PS_Copy.cso";
		desc.pRootSignature = mpRootSignature->Get();
		desc.RenderTargetNum = 1;
		mpPipeline = std::make_unique<Pipeline>(desc);
	}
	// ディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		// RTV用のヒープ設定
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = HEAP_NUM - 1;
		mpRTVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// レンダーターゲット
	{
		for (int i = 0; i < HEAP_NUM - 1; i++)
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.pRTVHeap = mpRTVHeap.get();
			desc.pSRVHeap = mpImGUIHeap.get();
			mpRTV.push_back(std::make_pair<bool, std::unique_ptr<RenderTarget>>(false, std::make_unique<RenderTarget>(desc)));
		}
	}

	return msg;
}

ImTextureID DebugImGUI::GetImGUIImage(DescriptorHeap* _heap, ConstantBuffer* _wvp, RenderTarget* _srv)
{
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	// 表示領域の設定
	D3D12_VIEWPORT vp = { 0, 0, 1280.0f, 720.0f, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, 1280.0f, 720.0f };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	// 使用するRTVを決定
	RenderTarget* useRTV = nullptr;
	for (int i = 0; i < mpRTV.size(); i++)
	{
		if (!mpRTV[i].first)
		{
			mpRTV[i].first = true;
			useRTV = mpRTV[i].second.get();
			break;
		}
	}
	if (!useRTV) return (ImTextureID)0;

	// レンダーターゲット切り替え
	useRTV->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		useRTV->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs);
	useRTV->Clear();

	// ImGUI用RTVに描画
	mpPipeline->Bind();
	D3D12_GPU_DESCRIPTOR_HANDLE hScreen[] = {
		_wvp->GetHandle().hGPU,
		_srv->GetHandleSRV().hGPU,
	};
	_heap->Bind();
	mpRootSignature->Bind(hScreen, _countof(hScreen));
	mpScreen->Draw();

	useRTV->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	return (ImTextureID)useRTV->GetHandleSRV().hGPU.ptr;
}

void DebugImGUI::CompletedDraw()
{
	for (int i = 0; i < mpRTV.size(); i++)
	{
		mpRTV[i].first = false;
	}
}

DescriptorHeap* DebugImGUI::GetImGUIDescriptorHeap()
{
	return mpImGUIHeap.get();
}
