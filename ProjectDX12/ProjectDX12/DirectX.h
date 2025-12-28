#ifndef ___DIRECTX_H___
#define ___DIRECTX_H___

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"DirectXMesh.lib")

#include <directx/d3dx12.h>

#include <cassert>

#define WINDOW_WIDTH (1920)
#define WINDOW_HEIGHT (1080)

#define SAFE_RELEASE(p) do {if(p){p->Release();p = nullptr;}} while(0)

using namespace Microsoft::WRL;

HRESULT InitDirectX(HWND hWnd, UINT width, UINT height, bool fullscreen);
void UninitDirectX();
void UpdateDirectX(void(func)(void));
void DrawDirectX(void(func)(void),const float clearColor[4]);

ID3D12Device8* GetDevice();
ID3D12GraphicsCommandList6* GetCommandList();
ID3D12CommandQueue* GetCommandQueue();
D3D12_CPU_DESCRIPTOR_HANDLE GetRTV();

void SetViewPort(float width, float height);
void SetRenderTarget(int num, D3D12_CPU_DESCRIPTOR_HANDLE * hRTV);
void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV);
void SetRenderTarget(int num, D3D12_CPU_DESCRIPTOR_HANDLE* hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV);

#endif
