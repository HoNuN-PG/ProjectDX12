
// ウィンドウの作成
// https://qiita.com/okmonn/items/94ea48e0fdcb2c74dbbf

#include "StartUp.h"

#include <Windows.h>
#include <tchar.h>
#include "Input.h"
#include "timer.h"

#include "DebugImGUI.h"

#include "SceneManager.h"
#include "DirectX.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

#pragma comment(lib,"winmm.lib")

std::unique_ptr<SceneManager> gSceneManager;
std::unique_ptr<DebugImGUI> gDebugImGUI;
std::unique_ptr<TimerFPS> gTimer;
std::unique_ptr<TimerFPS> gUpdateTimer;
std::unique_ptr<TimerFPS> gDrawTimer;

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

	// FPS出力
	float fps = gTimer->GetObsevationGameFPS(100);
	ImGui::Begin("GameFPS [ms]");
	ImGui::Text("fps:%5.3f", fps);
	ImGui::End();

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

	MSG msg = {};

	//---------------------------------
	// Init
	srand((unsigned int)timeGetTime());

	InitDirectX(hWnd,WINDOW_WIDTH,WINDOW_HEIGHT,false);

	gTimer = std::make_unique<TimerFPS>();
	gUpdateTimer = std::make_unique<TimerFPS>();
	gDrawTimer = std::make_unique<TimerFPS>();

	gDebugImGUI = std::make_unique<DebugImGUI>();
	msg = gDebugImGUI->Create(hWnd);

	gSceneManager = std::make_unique<SceneManager>();
	gSceneManager->Init();

	Input::Init();
	//---------------------------------

	timeBeginPeriod(1);

	// ゲームループ
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if(gTimer->CheckGameFPS()){
			Input::Update();

			// FPS計測
			gUpdateTimer->st = timeGetTime();
			gSceneManager->Update();
			gUpdateTimer->et = timeGetTime();
			float upf = gUpdateTimer->GetObservationDbFPS(1);

			gDrawTimer->st = timeGetTime();
			DrawDirectX(Draw, clear);
			gDrawTimer->et = timeGetTime();
			float drf = gDrawTimer->GetObservationDbFPS(1);

			gDebugImGUI->CompletedDraw();
		}
	}

	timeEndPeriod(1);

	// Uninit
	Input::Uninit();
	gSceneManager->Uninit();
	UninitDirectX();

	UnregisterClass(wc.lpszClassName,hInstance);
	return 0;
}
