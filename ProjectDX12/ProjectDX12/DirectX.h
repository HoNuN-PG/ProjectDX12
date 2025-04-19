#ifndef ___DIRECTX_H___
#define ___DIRECTX_H___

#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#define WINDOW_WIDTH (1280)
#define WINDOW_HEIGHT (720)

#define SAFE_RELEASE(p) do {if(p){p->Release();p = nullptr;}} while(0)

HRESULT InitDirectX(HWND hWnd, UINT width, UINT height, bool fullscreen);
void UninitDirectX();
void DrawDirectX(void(func)(void),const float clearColor[4]);

ID3D12Device* GetDevice();
ID3D12GraphicsCommandList* GetCommandList();
D3D12_CPU_DESCRIPTOR_HANDLE GetRTV();

void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV);
void SetRenderTarget(int num, D3D12_CPU_DESCRIPTOR_HANDLE* hRTV);
void SetRenderTarget(int num, D3D12_CPU_DESCRIPTOR_HANDLE* hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV);

#endif
