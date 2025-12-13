
#include <tchar.h>

// Debug
#include "DebugImGUI.h"

// ImGUI
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

//System/ConstantBuffer
#include "ConstantBuffer.h"
#include "ConstantWVP.h"

std::unique_ptr<MeshBuffer>									DebugImGUI::Screen;
std::unique_ptr<RootSignature>								DebugImGUI::RootSignatureData;
std::unique_ptr<Pipeline>									DebugImGUI::PipelineData;
std::vector<std::pair<bool, std::unique_ptr<RenderTarget>>>	DebugImGUI::ImGUIRTVs;
std::unique_ptr<DescriptorHeap>								DebugImGUI::ImGUIHeap;
std::unique_ptr<DescriptorHeap>								DebugImGUI::ImGUIRTVHeap;

DebugImGUI::DebugImGUI()
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
		ImGUIHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ImGUIの初期化
	// 実行ファイルの設定から高DPI項目の編集をする事でマウスポインタのズレ等を修正できる
	{
		if (ImGui::CreateContext() == nullptr) 
		{
			MessageBox(_hwnd, _T("Error"), _T("Failed [Imgui]."), MB_OK);
			msg.message = WM_QUIT;
		}
		else 
		{
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;

			if (!ImGui_ImplWin32_Init(_hwnd)) 
			{
				MessageBox(_hwnd, _T("Error"), _T("Failed [Imgui]."), MB_OK);
				msg.message = WM_QUIT;
			}
			else 
			{
				auto handle = ImGUIHeap->Allocate();
				ImGui_ImplDX12_Init(GetDevice(), 3,
					DXGI_FORMAT_R8G8B8A8_UNORM, ImGUIHeap->Get(), handle.hCPU, handle.hGPU);
			}
		}
	}
	// ドッキング設定
	{
		auto& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}
	// ルートシグネチャ
	{
		RootSignature::Parameter param[] = 
		{
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description desc = {};
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
		desc.pRootSignature = RootSignatureData->Get();
		desc.VSFile = L"../exe/assets/shader/VS_Sprite.cso";
		desc.PSFile = L"../exe/assets/shader/PS_Copy.cso";
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.RenderTargetNum = 1;
		desc.CullMode = D3D12_CULL_MODE_BACK;
		PipelineData = std::make_unique<Pipeline>(desc);
	}
	// ディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		// RTV用のヒープ設定
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = HEAP_NUM;
		ImGUIRTVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// レンダーターゲット
	{
		for (int i = 0; i < HEAP_NUM ; i++)
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.format = DXGI_FORMAT_B8G8R8A8_UNORM;
			desc.pRTVHeap = ImGUIRTVHeap.get();
			desc.pSRVHeap = ImGUIHeap.get();
			ImGUIRTVs.push_back(std::make_pair<bool, std::unique_ptr<RenderTarget>>(false, std::make_unique<RenderTarget>(desc)));
		}
	}

	return msg;
}

ImTextureID DebugImGUI::GetImGUIImage(DescriptorHeap* heap, RenderTarget* srv)
{
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();

	// 表示領域の設定
	D3D12_VIEWPORT vp = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	// 使用するRTVを決定
	RenderTarget* target = nullptr;
	for (int i = 0; i < ImGUIRTVs.size(); i++)
	{
		if (ImGUIRTVs[i].first) continue;

		ImGUIRTVs[i].first = true;
		target = ImGUIRTVs[i].second.get();
		break;
	}
	if (!target) return (ImTextureID)0;

	// レンダーターゲット切り替え
	target->SRV2RTV();
	target->Clear();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		target->GetHandleRTV().hCPU,
	};
	SetRenderTarget(1, rtvs);

	// ImGUI用RTVに描画
	ID3D12DescriptorHeap* heaps[] =
	{
		heap->Get(),
	};
	DescriptorHeap::Bind(heaps, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE handle[] = 
	{
		srv->GetHandleSRV().hGPU,
	};
	RootSignatureData->Bind(handle, _countof(handle));
	PipelineData->Bind();
	Screen->Draw();

	target->RTV2SRV();

	return (ImTextureID)target->GetHandleSRV().hGPU.ptr;
}

void DebugImGUI::Completed()
{
	for (int i = 0; i < ImGUIRTVs.size(); i++)
	{
		ImGUIRTVs[i].first = false;
	}
}

DescriptorHeap* DebugImGUI::GetImGUIDescriptorHeap()
{
	return ImGUIHeap.get();
}
