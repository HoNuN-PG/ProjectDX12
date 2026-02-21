
#include <tchar.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>

#include "imgui/imguiImage.h"
#include "System/Rendering/ConstantBuffer/ConstantBuffer.h"
#include "System/Rendering/ConstantBuffer/ConstantWVP.h"

std::unique_ptr<MeshBuffer>									ImGUIImage::pScreen;
std::unique_ptr<RootSignature>								ImGUIImage::pRootSignatureData;
std::unique_ptr<PipelineState>								ImGUIImage::pPipelineData;
std::vector<std::pair<bool, std::unique_ptr<RenderTarget>>>	ImGUIImage::Images;
std::unique_ptr<DescriptorHeap>								ImGUIImage::pHeap;
std::unique_ptr<DescriptorHeap>								ImGUIImage::pRTVHeap;

ImGUIImage::ImGUIImage()
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
	pScreen = std::make_unique<MeshBuffer>(desc);
}

ImGUIImage::~ImGUIImage()
{
}

MSG ImGUIImage::Create(HWND _hwnd)
{
	MSG msg = {};

	// ImGUI用のディスクリプタヒープを確保
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = HEAP_NUM + 1; // ImGUI用のヒープを追加
		pHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ImGUIの初期化
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
				auto handle = pHeap->Allocate();
				ImGui_ImplDX12_Init(
					GetDevice(), 
					3,
					DXGI_FORMAT_R8G8B8A8_UNORM, 
					pHeap->Get(), 
					handle.hCPU, 
					handle.hGPU
				);
				ImGuiIO& io = ImGui::GetIO();
				assert(io.Fonts != nullptr);
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
		pRootSignatureData = std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		PipelineState::Description desc = {};
		desc.VSFile = L"../game/assets/shader/VS_Sprite.cso";
		desc.PSFile = L"../game/assets/shader/PS_Copy.cso";
		desc.pRootSignature = pRootSignatureData->Get();
		desc.pInputLayout = PipelineState::IED_POS_TEX;
		desc.InputLayoutNum = PipelineState::IED_POS_TEX_COUNT;
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.RenderTargetNum = 1;
		pPipelineData = std::make_unique<PipelineState>(desc);
	}
	// RTV用のディスクリプタヒープを確保
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = HEAP_NUM;
		pRTVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// レンダーターゲット
	{
		for (int i = 0; i < HEAP_NUM ; i++)
		{
			RenderTarget::Description desc = {};
			desc.width = WINDOW_WIDTH;
			desc.height = WINDOW_HEIGHT;
			desc.format = DXGI_FORMAT_B8G8R8A8_UNORM;
			desc.pRTVHeap = pRTVHeap.get();
			desc.pSRVHeap = pHeap.get();
			Images.push_back(std::make_pair<bool, std::unique_ptr<RenderTarget>>(false, std::make_unique<RenderTarget>(desc)));
		}
	}

	return msg;
}

ImTextureID ImGUIImage::GetImage(DescriptorHeap* heap, RenderTarget* srv)
{
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();

	// 表示領域の設定
	D3D12_VIEWPORT vp = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	// 使用するRTVを決定
	RenderTarget* target = nullptr;
	for (int i = 0; i < Images.size(); i++)
	{
		if (Images[i].first) continue;

		Images[i].first = true;
		target = Images[i].second.get();
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
	pRootSignatureData->Bind(handle, _countof(handle));
	pPipelineData->Bind();
	pScreen->Draw();

	target->RTV2SRV();

	return (ImTextureID)target->GetHandleSRV().hGPU.ptr;
}

void ImGUIImage::Completed()
{
	for (int i = 0; i < Images.size(); i++)
	{
		Images[i].first = false;
	}
}
