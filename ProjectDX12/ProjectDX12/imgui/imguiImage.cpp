
#include <tchar.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>

#include "imgui/imguiImage.h"
#include "System/DirectX.h"

ImGUIImage* ImGUIImage::instance = nullptr;

ImGUIImage* ImGUIImage::GetInstance()
{
	if (!instance)
	{
		instance = new ImGUIImage();
	}
	return instance;
}

void ImGUIImage::Destroy()
{
	if (!instance) return;

	instance->Uninit();
	delete instance;
	instance = nullptr;
}

bool ImGUIImage::Init(HWND hwnd)
{
	// ImGUI用のディスクリプタヒープを確保
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = IMAGE_MAX + 1; // フォント用を追加
		pHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// RTV用のディスクリプタヒープを確保
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = IMAGE_MAX;
		pRTVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// ImGUIの初期化
	{
		if (ImGui::CreateContext() == nullptr)
		{
			MessageBox(hwnd, _T("Failed [Imgui]."), _T("Error"), MB_OK);
			return false;
		}

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // ドッキング設定

		if (!ImGui_ImplWin32_Init(hwnd))
		{
			MessageBox(hwnd, _T("Failed [Imgui]."), _T("Error"), MB_OK);
			return false;
		}

		DescriptorHeap::Handle font = pHeap->Allocate();
		if (!ImGui_ImplDX12_Init(
			GetDevice(),
			BACKBUFFER_COUNT,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			pHeap->Get(),
			font.hCPU,
			font.hGPU))
		{
			MessageBox(hwnd, _T("Failed [Imgui]."), _T("Error"), MB_OK);
			return false;
		}
	}
	// スクリーン
	{
		ScreenVertex screenVtx[] =
		{
			{{-0.5f, 0.5f,0} ,{0,0}} ,
			{{ 0.5f, 0.5f,0} ,{1,0}} ,
			{{-0.5f,-0.5f,0} ,{0,1}} ,
			{{ 0.5f,-0.5f,0} ,{1,1}} ,
		};

		MeshBuffer::Description desc = {};
		desc.pVtx = screenVtx;
		desc.vtxSize = sizeof(ScreenVertex);
		desc.vtxCount = _countof(screenVtx);
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		pScreen = std::make_unique<MeshBuffer>(desc);
	}
	// ルートシグネチャ
	{
		RootSignature::Parameter param[] =
		{
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description rootsignature =
		{
			param,
			_countof(param)
		};
		pRootSignatureData = std::make_unique<RootSignature>(rootsignature);
	}
	// パイプライン
	{
		std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT_B8G8R8A8_UNORM };
		PipelineState::Description desc = {
			L"",
			L"",
			L"../game/assets/shader/VS_Sprite.cso",
			L"../game/assets/shader/PS_Copy.cso",
			pRootSignatureData->Get(),
			PipelineState::IED_POS_TEX,
			PipelineState::IED_POS_TEX_COUNT,
			D3D12_CULL_MODE_BACK,
			1,
			formats,
			FALSE,
		};
		pPipelineData = std::make_unique<PipelineState>(desc);
	}

	return true;
}

void ImGUIImage::Uninit()
{
	// 初期化に成功した段階まで終了処理を行う
	if (ImGui::GetCurrentContext())
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.BackendRendererUserData)
		{
			ImGui_ImplDX12_Shutdown();
		}
		if (io.BackendPlatformUserData)
		{
			ImGui_ImplWin32_Shutdown();
		}
		ImGui::DestroyContext();
	}

	Images.clear();
	UsedImageCount = 0;
	pPipelineData.reset();
	pRootSignatureData.reset();
	pScreen.reset();
	pRTVHeap.reset();
	pHeap.reset();
}

void ImGUIImage::NewFrame()
{
	// 前フレームの表示画像を解放
	UsedImageCount = 0;

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGUIImage::Render()
{
	ImGui::Render();

	ID3D12DescriptorHeap* heaps[] =
	{
		pHeap->Get(),
	};
	DescriptorHeap::Bind(heaps, 1);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GetCommandList());
}

ImTextureID ImGUIImage::GetImage(DescriptorHeap* heap, RenderTarget* srv)
{
	return GetImage(heap, srv->GetHandleSRV().hGPU);
}

ImTextureID ImGUIImage::GetImage(DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE srv)
{
	// 使用する表示画像を決定（足りなければ作成）
	if (UsedImageCount >= IMAGE_MAX) return (ImTextureID)0;
	
	if (UsedImageCount == Images.size())
	{
		RenderTarget::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.pRTVHeap = pRTVHeap.get();
		desc.pSRVHeap = pHeap.get();
		Images.push_back(std::make_unique<RenderTarget>(desc));
	}
	RenderTarget* target = Images[UsedImageCount++].get();

	// 表示領域の設定
	SetViewPort(target->Width, target->Height);

	// ImGUI用RTVに描画
	std::vector<RenderTarget*> targets = { target };
	D3D12_GPU_DESCRIPTOR_HANDLE handle[] =
	{
		srv,
	};
	ScreenDraw(targets, heap->Get(), handle, _countof(handle), pRootSignatureData.get(), pPipelineData.get(), pScreen.get());

	return (ImTextureID)target->GetHandleSRV().hGPU.ptr;
}
