
#include "StartUp.h"

#include <Windows.h>
#include <tchar.h>

#include "DirectX.h"
#include "Scene.h"
#include "Input.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

#pragma comment(lib,"winmm.lib")

#include "SceneManager.h"

std::unique_ptr<SceneManager> gSceneManager;

std::shared_ptr<DescriptorHeap> gpHeapImGUI;
std::shared_ptr<DescriptorHeap> GetHeapImGUI()
{
	return gpHeapImGUI;
}
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT WinProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wparam, lparam))
		return true;
	switch (msg)
	{
	default: break;
	case(WM_DESTROY):
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd,msg,wparam,lparam);
}

void Draw()
{
#ifdef _DEBUG
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif

	gSceneManager->Draw();

#ifdef _DEBUG
	ImGui::Render();
	ID3D12DescriptorHeap* heap = gpHeapImGUI->Get();
	GetCommandList()->SetDescriptorHeaps(1, &heap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GetCommandList());
#endif
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPUTSTR lpCmdLine, int nCmdShow)
{
	// ウィンドウクラス
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = reinterpret_cast<WNDPROC>(WinProc);
	wc.lpszClassName = _T("DirectX12");
	wc.hInstance = hInstance;
	RegisterClassEx(&wc);

	// ウィンドウの作成
	RECT wndRect = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };
	// ウィンドウサイズを調整
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&wndRect, style, FALSE);
	HWND hWnd = CreateWindow(
		wc.lpszClassName, _T("DirectX12"),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,
		wndRect.right - wndRect.left,wndRect.bottom - wndRect.top,NULL,NULL,hInstance,NULL
	);
	ShowWindow(hWnd,nCmdShow);

	// ClearColor
	float clear[] = { 0.0f,0.0f,0.0f,1 };

	// Init
	srand((unsigned int)timeGetTime());
	InitDirectX(hWnd,WINDOW_WIDTH,WINDOW_HEIGHT,false);

	MSG msg = {};

	// ImGUI用のディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 5;
		gpHeapImGUI = std::make_shared<DescriptorHeap>(desc);
	}
	// ImGUIの初期化
	// 実行ファイルの設定から高DPI項目の編集をする事でマウスポインタのズレ等を修正できる
	{
		if (ImGui::CreateContext() == nullptr) {
			MessageBox(hWnd, _T("Error"), _T("Failed [Imgui]."), MB_OK);
			msg.message = WM_QUIT;
		}
		else {
			// Windows用のimguiの初期化
			if (!ImGui_ImplWin32_Init(hWnd)) {
				MessageBox(hWnd, _T("Error"), _T("Failed [Imgui]."), MB_OK);
				msg.message = WM_QUIT;
			}
			else {
				// DirectX用のimguiの初期化
				auto handle = gpHeapImGUI->Allocate();
				ImGui_ImplDX12_Init(GetDevice(), 3,
					DXGI_FORMAT_R8G8B8A8_UNORM, gpHeapImGUI->Get(), handle.hCPU, handle.hGPU);
			}
		}
	}

	gSceneManager = std::make_unique<SceneManager>();
	gSceneManager->Init();
	Input::Init();

	// ゲームループ
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			Input::Update();
			gSceneManager->Update();
			DrawDirectX(Draw, clear);
		}
	}

	// Uninit
	Input::Uninit();
	gSceneManager->Uninit();
	gSceneManager.release();
	UninitDirectX();

	UnregisterClass(wc.lpszClassName,hInstance);
	return 0;
}
