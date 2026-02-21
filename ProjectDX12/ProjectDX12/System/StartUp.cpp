
#include <tchar.h>
#include <Windows.h>
#pragma comment(lib,"winmm.lib")
#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")

#include "imguiImage.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>
#include "Scene/SceneManager.h"
#include "System/DirectX.h"
#include "System/Input.h"
#include "System/StartUp.h"
#include "System/timer.h"

HWND gWindow;
bool bBorderlessed{ false };
std::unique_ptr<TimerFPS> gTimer;
std::unique_ptr<TimerFPS> gUpdateTimer;
std::unique_ptr<TimerFPS> gDrawTimer;
std::unique_ptr<ImGUIImage> gImGUIImage;
std::unique_ptr<SceneManager> gSceneManager;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
LRESULT WinProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);
void SetBorderless(HWND hwnd, bool enabled);
void Update();
void Draw();

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPUTSTR lpCmdLine, int nCmdShow)
{
	// 高DPI対応
	ImGui_ImplWin32_EnableDpiAwareness();

	//-----------------
	// ウィンドウ作成
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = reinterpret_cast<WNDPROC>(WinProc);
	wc.lpszClassName = _T("DirectX12");
	wc.hInstance = hInstance;
	RegisterClassEx(&wc);

	// ウィンドウサイズを調整
	RECT wndRect = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };
	auto style = WS_OVERLAPPEDWINDOW;
	AdjustWindowRect(&wndRect, style, FALSE);
	gWindow = CreateWindow(
		wc.lpszClassName, _T("DirectX12"),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,
		wndRect.right - wndRect.left,wndRect.bottom - wndRect.top,NULL,NULL,hInstance,NULL
	);
	// ウィンドウスタイル変更
	SetBorderless(gWindow, bBorderlessed = true);
	//-----------------

	float clear[] = { 0.0f,0.0f,0.0f,1 };
	MSG msg = {};

	//---------------------------------
	// Init
	// 乱数初期化
	srand((unsigned int)timeGetTime());

	// DirectX初期化
	InitDirectX(gWindow,WINDOW_WIDTH,WINDOW_HEIGHT,false);

	// ウィンドウ表示
	ShowWindow(gWindow, nCmdShow);

	// 計測タイマー初期化
	gTimer = std::make_unique<TimerFPS>();
	gUpdateTimer = std::make_unique<TimerFPS>();
	gDrawTimer = std::make_unique<TimerFPS>();

	// デバッグ用IｍGUI初期化
	gImGUIImage = std::make_unique<ImGUIImage>();
	gImGUIImage->Create(gWindow);

	// 入力初期化
	Input::Init();

	// シーン初期化
	gSceneManager = std::make_unique<SceneManager>();
	gSceneManager->Init();
	//---------------------------------

	timeBeginPeriod(1);

	//------------------------------
	// ゲームループ
	while (msg.message != WM_QUIT) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if(gTimer->CheckGameFPS())
		{
			// 入力更新
			Input::Update();

			// 更新
			gUpdateTimer->st = timeGetTime();
			UpdateDirectX(Update);
			gUpdateTimer->et = timeGetTime();
			float upf = gUpdateTimer->GetObservationDbFPS(1);

			// 描画
			gDrawTimer->st = timeGetTime();
			DrawDirectX(Draw, clear);
			gDrawTimer->et = timeGetTime();
			float drf = gDrawTimer->GetObservationDbFPS(1);

			gImGUIImage->Completed();
		}
	}
	//------------------------------

	timeEndPeriod(1);

	//--------------
	// Uninit
	gSceneManager->Uninit();
	Input::Uninit();
	UninitDirectX();
	//--------------

	UnregisterClass(wc.lpszClassName,hInstance);
	return 0;
}

LRESULT WinProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wparam, lparam))
	{
		return true;
	}

	switch (msg)
	{
	default:
		break;
	case(WM_DESTROY):
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch (wparam)
		{
		case VK_ESCAPE:
			int id = MessageBox(NULL, TEXT("プログラムを終了しますか？"), TEXT("App"), MB_OKCANCEL | MB_ICONQUESTION);
			switch (id)
			{
			case IDOK:
				DestroyWindow(hWnd);
				break;
			case IDCANCEL:
				break;
			}
			break;
		}
		break;
	case WM_SIZE:
		switch (wparam)
		{
		case(SIZE_MAXIMIZED):
			if (!bBorderlessed)
			{
				SetBorderless(gWindow, bBorderlessed = true);
			}
			break;
		case(SIZE_MINIMIZED):
			if (bBorderlessed)
			{
				SetBorderless(gWindow, bBorderlessed = false);
			}
			break;
		}
		break;
	}
	return DefWindowProc(hWnd, msg, wparam, lparam);
}

void SetBorderless(HWND hwnd, bool enabled)
{
	if (enabled)
	{
		// 現在のモニター
		HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hMon, &mi);

		// 装飾を削除
		LONG style = GetWindowLong(hwnd, GWL_STYLE);
		style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
		SetWindowLong(hwnd, GWL_STYLE, style);

		LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
		exStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
		SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

		// モニター全体にフィット
		SetWindowPos(
			hwnd,
			HWND_TOP,
			mi.rcMonitor.left,
			mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_SHOWWINDOW
		);
	}
	else
	{
		// 元に戻す
		LONG style = WS_OVERLAPPEDWINDOW;
		SetWindowLong(hwnd, GWL_STYLE, style);

		LONG exStyle = WS_EX_OVERLAPPEDWINDOW;
		SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

		RECT rc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };
		AdjustWindowRect(&rc, style, FALSE);

		SetWindowPos(hwnd, HWND_TOP, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
	}
}

void Update()
{
	// シーン更新
	gSceneManager->Update();
}

void Draw()
{
	// ImGUI開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// シーン描画
	gSceneManager->Draw();

	// FPS出力
	float fps = gTimer->GetObsevationGameFPS(100);
	ImGui::Begin("GameFPS [ms]");
	ImGui::Text("fps:%5.3f", fps);
	ImGui::End();

	// ImGUI終了
	ImGui::Render();
	ID3D12DescriptorHeap* heap = gImGUIImage->GetImGUIDescriptorHeap()->Get();
	GetCommandList()->SetDescriptorHeaps(1, &heap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GetCommandList());
}