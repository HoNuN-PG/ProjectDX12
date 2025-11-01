
#include "DirectX.h"

ID3D12Device*				g_pDevice;			// DirectX12デバイスオブジェクト
IDXGIFactory6*				g_pFactory;			// ディスプレイへの出力機能の制御
ID3D12CommandAllocator*		g_pCmdAllocator;	// 描画命令を蓄積
ID3D12GraphicsCommandList*	g_pCmdList;			// 描画命令発行インターフェース
ID3D12CommandQueue*			g_pCmdQueue;		// 描画命令の実行
IDXGISwapChain3*			g_pSwapChain;		// ダブルバッファリングを行う
ID3D12DescriptorHeap*		g_RTVHeap;			// 描画先として紐づけるヒープ
ID3D12Resource**			g_ppBackBuf;		// バックバッファ
UINT64						g_fenceLevel;
ID3D12Fence*				g_pFence;			// フェンス
D3D12_VIEWPORT				g_viewPort;			// ビューポート
D3D12_RECT					g_scissor;			// シザー

HRESULT InitDirectX(HWND hWnd, UINT width, UINT height, bool fullscreen)
{
	// デバッグ設定
#ifdef _DEBUG
	ID3D12Debug* pDebug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug)))) {
		pDebug->EnableDebugLayer();
	}
#endif

	HRESULT hr;
#ifdef _DEBUG
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&g_pFactory));
#else
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&g_pFactory));
#endif
	if (FAILED(hr)) { return hr; }

#if 1
	Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;	//デバイス取得用
	int adapterIndex = 0;											//列挙するデバイスのインデックス
	bool adapterFound = false;										//目的のデバイスを見つけたか

	// 目的のデバイスを探索
	while (g_pFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND) 
	{
		DXGI_ADAPTER_DESC1 desc;
		dxgiAdapter->GetDesc1(&desc);  // デバイスの情報を取得

		// ハードウェアのみ選ぶ
		if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) 
		{
			D3D_FEATURE_LEVEL featureLevels[] = 
			{
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0,
			};
			bool succeeded = false;
			for (auto lv : featureLevels) 
			{
				hr = D3D12CreateDevice(dxgiAdapter.Get(),lv, IID_PPV_ARGS(&g_pDevice));
				if (SUCCEEDED(hr)) 
				{ 
					succeeded = true;
					break; 
				}
			}
		}
		++adapterIndex;
	}
#else
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
	};
	// ハードウェアの機能レベルを指定してデバイス初期化
	for (auto lv : featureLevels) {
		hr = D3D12CreateDevice(nullptr,lv,IID_PPV_ARGS(&g_pDevice));
		if (SUCCEEDED(hr)) { break; }
	}
	if (FAILED(hr)) { return hr; }
#endif

	// コマンドリスト/コマンドキュー/コマンドアロケーター
	// コマンドの種類
	D3D12_COMMAND_LIST_TYPE cmdListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
	
	hr = g_pDevice->CreateCommandAllocator(cmdListType, IID_PPV_ARGS(&g_pCmdAllocator));
	if (FAILED(hr)) { return hr; }

	hr = g_pDevice->CreateCommandList(0,cmdListType,g_pCmdAllocator,nullptr,
		IID_PPV_ARGS(&g_pCmdList));
	if (FAILED(hr)) { return hr; }

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type		= cmdListType;
	cmdQueueDesc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	// コマンドキュ―の優先順位
	cmdQueueDesc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;		// GPUのタイムアウト有効
	cmdQueueDesc.NodeMask	= 0;									// 複数のGPUの場合に、適用するGPUを識別するビット
	hr = g_pDevice->CreateCommandQueue(&cmdQueueDesc,IID_PPV_ARGS(&g_pCmdQueue));
	if (FAILED(hr)) { return hr; }

	// スワップチェーン
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	// 解像度
	scDesc.Width				= width;
	scDesc.Height				= height;
	scDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo				= false;
	// ピクセルあたりのサンプル数
	scDesc.SampleDesc.Count		= 1;
	scDesc.SampleDesc.Quality	= 0;
	// バックバッファのフラグ
	scDesc.BufferUsage			= DXGI_USAGE_BACK_BUFFER;
	scDesc.BufferCount			= 2;
	// バッファ出力とターゲット出力の一致の為サイズ変更を行う
	scDesc.Scaling				= DXGI_SCALING_STRETCH;
	scDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.AlphaMode			= DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.Flags				= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	hr = g_pFactory->CreateSwapChainForHwnd(
		g_pCmdQueue,hWnd,&scDesc,nullptr,nullptr,
		reinterpret_cast<IDXGISwapChain1**>(&g_pSwapChain)
	);
	if (FAILED(hr)) { return hr; }

	// ディスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC rtvDHDesc = {};
	// ディスクリプタヒープの種類
	rtvDHDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDHDesc.NumDescriptors	= 2;
	rtvDHDesc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDHDesc.NodeMask			= 0;
	hr = g_pDevice->CreateDescriptorHeap(&rtvDHDesc,IID_PPV_ARGS(&g_RTVHeap));
	if (FAILED(hr)) { return hr; }

	// レンダーターゲットビューのディスクリプタを作成
	g_ppBackBuf = new ID3D12Resource*[scDesc.BufferCount];
	D3D12_CPU_DESCRIPTOR_HANDLE handle = g_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < scDesc.BufferCount; ++i)
	{
		// ディスプレイに紐づいたリソースから、ディスクリプタを作成
		hr = g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_ppBackBuf[i]));
		g_pDevice->CreateRenderTargetView(g_ppBackBuf[i], nullptr, handle);

		// ヒープ内の次の位置へ移動
		handle.ptr += g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// フェンス
	g_fenceLevel = 0;
	hr = g_pDevice->CreateFence(g_fenceLevel,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&g_pFence));
	if (FAILED(hr)) { return hr; }

	return hr;
}

void UninitDirectX()
{
	DXGI_SWAP_CHAIN_DESC1 scDesc;
	g_pSwapChain->GetDesc1(&scDesc);
	for(UINT i = 0;i < scDesc.BufferCount;++i)
	{
		SAFE_RELEASE(g_ppBackBuf[i]);
	}
	delete g_ppBackBuf;
	g_ppBackBuf = nullptr;
	SAFE_RELEASE(g_RTVHeap);
	SAFE_RELEASE(g_pCmdQueue);
	SAFE_RELEASE(g_pCmdList);
	SAFE_RELEASE(g_pCmdAllocator);
	SAFE_RELEASE(g_pDevice);
}

void DrawDirectX(void(func)(void), const float clearColor[4])
{
	// 描画準備
	UINT bbIdx = g_pSwapChain->GetCurrentBackBufferIndex(); // 現在のバックバッファ
	g_pCmdAllocator->Reset();								// コマンドアロケーターのクリア
	g_pCmdList->Reset(g_pCmdAllocator, nullptr);			// コマンドリストのクローズを解除して再び描画命令を蓄積

	// バックバッファをレンダーターゲットとして利用
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	barrierDesc.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;	// 使用方法の移行
	barrierDesc.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;			// フラグ無し
	barrierDesc.Transition.pResource	= g_ppBackBuf[bbIdx];						// 移行するリソース
	barrierDesc.Transition.Subresource	= 0;
	barrierDesc.Transition.StateBefore	= D3D12_RESOURCE_STATE_PRESENT;				// 以前の使用方法
	barrierDesc.Transition.StateAfter	= D3D12_RESOURCE_STATE_RENDER_TARGET;		// 移行後の使用方法
	g_pCmdList->ResourceBarrier(1,&barrierDesc);

	// レンダーターゲット設定
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV;
	hRTV = g_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	hRTV.ptr += bbIdx * g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	g_pCmdList->OMSetRenderTargets(
		1,
		&hRTV,
		false,
		nullptr
	);
	// レンダーターゲットのクリア
	g_pCmdList->ClearRenderTargetView(hRTV, clearColor, 0, nullptr);

	// 描画
	ID3D12CommandList* pCmdList[] = { g_pCmdList };
	func();

	// 画面出力準備
	barrierDesc.Transition.StateBefore	= D3D12_RESOURCE_STATE_RENDER_TARGET;
	// 画面への出力リソース
	barrierDesc.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
	g_pCmdList->ResourceBarrier(1, &barrierDesc);

	// 描画命令発行停止
	g_pCmdList->Close();							// コマンドリストからの描画命令の発行を停止
	g_pCmdQueue->ExecuteCommandLists(1, pCmdList);	// コマンドリストに紐づくアロケータから描画命令を送信

	// 画面出力
	g_pSwapChain->Present(1, 0);

	// コマンドリストの完了をチェック
	g_pCmdQueue->Signal(g_pFence, ++g_fenceLevel);
	if (g_pFence->GetCompletedValue() != g_fenceLevel)
	{
		// Windowsのイベントで処理を待つ
		auto hWait = CreateEvent(nullptr, false, false, nullptr);
		g_pFence->SetEventOnCompletion(g_fenceLevel, hWait);		// 終了値に到達した際に通知するイベントを設定
		WaitForSingleObject(hWait, INFINITE);						// イベントが発行されるまで待機
		CloseHandle(hWait);											// イベントの破棄
	}
}

ID3D12Device * GetDevice()
{
	return g_pDevice;
}

ID3D12GraphicsCommandList * GetCommandList()
{
	return g_pCmdList;
}

D3D12_CPU_DESCRIPTOR_HANDLE GetRTV()
{
	UINT bbIdx = g_pSwapChain->GetCurrentBackBufferIndex();
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV;
	hRTV = g_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	hRTV.ptr += bbIdx * g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return hRTV;
}

void SetViewPort(float width, float height)
{
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	// 表示領域の設定
	D3D12_VIEWPORT vp = { 0, 0, width, height, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, width, height };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);
}

void SetRenderTarget(int num, D3D12_CPU_DESCRIPTOR_HANDLE* hRTV)
{
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	pCmdList->OMSetRenderTargets(num, hRTV, FALSE, nullptr);
}

void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV)
{
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	pCmdList->OMSetRenderTargets(1, &hRTV, FALSE, hDSV.ptr ? &hDSV : nullptr);
}

void SetRenderTarget(int num, D3D12_CPU_DESCRIPTOR_HANDLE* hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV)
{
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	pCmdList->OMSetRenderTargets(num, hRTV, FALSE, hDSV.ptr ? &hDSV : nullptr);
}
