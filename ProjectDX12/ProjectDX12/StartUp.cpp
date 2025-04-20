
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

std::unique_ptr<DebugImGUI> gDebugImGUI;
DebugImGUI* GetDebugImGUI()
{
	return gDebugImGUI.get();
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
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	gSceneManager->Draw();

	ImGui::Render();
	ID3D12DescriptorHeap* heap = gDebugImGUI->GetImGUIDescriptorHeap()->Get();
	GetCommandList()->SetDescriptorHeaps(1, &heap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GetCommandList());
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
	gDebugImGUI = std::make_unique<DebugImGUI>();
	MSG msg = gDebugImGUI->Create(hWnd);
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
			gDebugImGUI->CompletedDraw();
		}
	}

	// Uninit
	Input::Uninit();
	gSceneManager->Uninit();
	UninitDirectX();

	UnregisterClass(wc.lpszClassName,hInstance);
	return 0;
}
