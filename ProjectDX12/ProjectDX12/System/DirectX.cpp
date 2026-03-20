
#include "System/DirectX.h"

#define BACKBUFFER_COUNT (2)											// バックバッファの数

ComPtr<ID3D12Device8>				g_pDevice;							// DirectX12デバイスオブジェクト
ComPtr<IDXGIFactory6>				g_pFactory;							// ディスプレイへの出力機能の制御
ComPtr<ID3D12CommandAllocator>		g_pCmdAllocator[BACKBUFFER_COUNT];	// コマンドアロケータ
ComPtr<ID3D12GraphicsCommandList6>	g_pCmdList;							// コマンドリスト
ComPtr<ID3D12CommandQueue>			g_pCmdQueue;						// コマンドキュー
ComPtr<IDXGISwapChain3>				g_pSwapChain;
ComPtr<ID3D12DescriptorHeap>		g_BBufferHeap;						// バックバッファヒープ
ComPtr<ID3D12Resource>				g_pBackBuf[BACKBUFFER_COUNT];		// バックバッファ
UINT64								g_BackBufferIdx;					// バックバッファのインデックス	
UINT64								g_fenceLevel[BACKBUFFER_COUNT];
ComPtr<ID3D12Fence>					g_pFence;							// フェンス

HRESULT InitDirectX(HWND hWnd, UINT width, UINT height, bool fullscreen)
{
#ifdef _DEBUG
	// デバッグ設定
	ID3D12Debug* pDebug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug)))) 
	{
		pDebug->EnableDebugLayer();
	}
#endif

	HRESULT hr;

	// ファクトリ作成
#ifdef _DEBUG
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(g_pFactory.ReleaseAndGetAddressOf()));
#else
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&g_pFactory));
#endif
	if (FAILED(hr)) { return hr; }
	
	// デバイス作成
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	hr = g_pFactory->EnumAdapterByGpuPreference(
		0,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&adapter)
	);
	if (SUCCEEDED(hr))
	{
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
		};
		for (auto lv : featureLevels)
		{
			hr = D3D12CreateDevice(
				adapter.Get(),
				lv,
				IID_PPV_ARGS(g_pDevice.ReleaseAndGetAddressOf())
			);
			if (SUCCEEDED(hr))
			{
				break;
			}
		}
	}

	// MeshShaderのサポートを確認
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
	hr = g_pDevice->CheckFeatureSupport(
		D3D12_FEATURE_D3D12_OPTIONS7,
		&options7,
		sizeof(options7)
	);
	if (FAILED(hr) || options7.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)
	{
		return hr;
	}

	// コマンドの種類
	D3D12_COMMAND_LIST_TYPE cmdListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// コマンドキュー
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type		= cmdListType;
	cmdQueueDesc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	// コマンドキュ―の優先順位
	cmdQueueDesc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;		// GPUのタイムアウト有効
	cmdQueueDesc.NodeMask	= 0;									// 複数のGPUの場合に、適用するGPUを識別するビット
	hr = g_pDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(g_pCmdQueue.ReleaseAndGetAddressOf()));
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
	scDesc.BufferCount			= BACKBUFFER_COUNT;
	// バッファ出力とターゲット出力の一致の為サイズ変更を行う
	scDesc.Scaling				= DXGI_SCALING_STRETCH;
	scDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.AlphaMode			= DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.Flags				= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	hr = g_pFactory->CreateSwapChainForHwnd(
		g_pCmdQueue.Get(),
		hWnd,
		&scDesc,
		nullptr,
		nullptr,
		reinterpret_cast<IDXGISwapChain1**>(g_pSwapChain.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr)) { return hr; }

	// バックバッファインデックス
	g_BackBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();

	// コマンドアロケーター
	for (UINT i = 0; i < BACKBUFFER_COUNT; ++i)
	{
		hr = g_pDevice->CreateCommandAllocator(
			cmdListType,
			IID_PPV_ARGS(g_pCmdAllocator[i].ReleaseAndGetAddressOf())
		);
	}
	if (FAILED(hr)) { return hr; }

	// コマンドリスト
	hr = g_pDevice->CreateCommandList(
		0,
		cmdListType,
		g_pCmdAllocator[g_BackBufferIdx].Get(),
		nullptr,
		IID_PPV_ARGS(g_pCmdList.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr)) { return hr; }
	g_pCmdList->Close();

	// ディスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC rtvDHDesc = {};
	rtvDHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDHDesc.NumDescriptors = BACKBUFFER_COUNT;
	rtvDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDHDesc.NodeMask = 0;
	hr = g_pDevice->CreateDescriptorHeap(&rtvDHDesc, IID_PPV_ARGS(g_BBufferHeap.GetAddressOf()));
	if (FAILED(hr)) { return hr; }

	// レンダーターゲットビューのディスクリプタを作成
	D3D12_CPU_DESCRIPTOR_HANDLE handle = g_BBufferHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < BACKBUFFER_COUNT; ++i)
	{
		// ディスプレイに紐づいたリソースから、ディスクリプタを作成
		hr = g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(g_pBackBuf[i].ReleaseAndGetAddressOf()));
		g_pDevice->CreateRenderTargetView(g_pBackBuf[i].Get(), nullptr, handle);

		// ヒープ内の次の位置へ移動
		handle.ptr += g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// フェンス
	hr = g_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(g_pFence.GetAddressOf()));
	if (FAILED(hr)) { return hr; }
	g_fenceLevel[g_BackBufferIdx] = 0;

	return hr;
}

void UninitDirectX()
{
}

void BeginRendering()
{
	float clearColor[4] = { 0, 0, 0, 0 };

	g_pCmdAllocator[g_BackBufferIdx]->Reset();										// コマンドアロケーターのクリア
	g_pCmdList->Reset(g_pCmdAllocator[g_BackBufferIdx].Get(), nullptr);				// コマンドリストのクリア

	// バックバッファに対するリソースバリア
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Transition.pResource = g_pBackBuf[g_BackBufferIdx].Get();
	barrierDesc.Transition.Subresource = 0;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	g_pCmdList->ResourceBarrier(1, &barrierDesc);

	// レンダーターゲット設定
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV;
	hRTV = g_BBufferHeap->GetCPUDescriptorHandleForHeapStart();
	hRTV.ptr += g_BackBufferIdx * g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	SetRenderTarget(1, &hRTV);
	g_pCmdList->ClearRenderTargetView(hRTV, clearColor, 0, nullptr);
}

void EndRendering()
{
	// 描画
	ID3D12CommandList* pCmdList[] = { g_pCmdList.Get() };

	// バックバッファに対するリソースバリア
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Transition.pResource = g_pBackBuf[g_BackBufferIdx].Get();
	barrierDesc.Transition.Subresource = 0;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	g_pCmdList->ResourceBarrier(1, &barrierDesc);

	// 描画命令発行停止
	g_pCmdList->Close();							// コマンドリストの停止
	g_pCmdQueue->ExecuteCommandLists(1, pCmdList);	// コマンドリストの内容を発行

	// 画面出力
	g_pSwapChain->Present(1, 0);

	// フェンス値更新
	UINT64 fence = ++g_fenceLevel[g_BackBufferIdx];

	// シグナル
	g_pCmdQueue->Signal(g_pFence.Get(), fence);

	// コマンドリストの完了をチェック
	if (g_pFence->GetCompletedValue() < fence)
	{
		// Windowsのイベントで処理を待つ
		auto event = CreateEvent(nullptr, false, false, nullptr);
		g_pFence->SetEventOnCompletion(fence, event); // 終了値に到達した際に通知するイベントを設定
		WaitForSingleObject(event, INFINITE); // イベントが発行されるまで待機
		CloseHandle(event);	// イベントの破棄
	}

	// バックバッファインデックス
	g_BackBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
}

ID3D12Device8 * GetDevice()
{
	return g_pDevice.Get();
}

ID3D12GraphicsCommandList6 * GetCommandList()
{
	return g_pCmdList.Get();
}

ID3D12CommandQueue* GetCommandQueue()
{
	return g_pCmdQueue.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE GetBBuffer()
{
	UINT bbIdx = g_pSwapChain->GetCurrentBackBufferIndex();
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV;
	hRTV = g_BBufferHeap->GetCPUDescriptorHandleForHeapStart();
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
