#ifndef __DEBUG_IMGUI_H__
#define __DEBUG_IMGUI_H__

#include <vector>
#include <memory>
#include <DirectXMath.h>

#include "imgui/imgui.h"

#include "MeshBuffer.h"
#include "DescriptorHeap.h"

class ConstantBuffer;
class RootSignature;
class Pipeline;
class RenderTarget;

class DebugImGUI
{
public:
	struct Vertex {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT4 color;
	};

private:
	const int HEAP_NUM = 5;

public:
	DebugImGUI();
	~DebugImGUI();
	MSG Create(HWND _hwnd);

public:
	/// <summary>
	/// ImGUI::Imageで表示するリソースの取得
	/// </summary>
	/// <param name="_heap">_wvpと_srvに使用されているヒープ</param>
	/// <param name="_wvp"></param>
	/// <param name="_srv"></param>
	/// <returns></returns>
	ImTextureID GetImGUIImage(DescriptorHeap* _heap, ConstantBuffer* _wvp, RenderTarget* _srv);
	// 描画終了時に呼び出し
	void CompletedDraw();

private:
	std::unique_ptr<MeshBuffer> mpScreen;
	std::unique_ptr<RootSignature>								mpRootSignature;
	std::unique_ptr<Pipeline>									mpPipeline;
	std::vector<std::pair<bool,std::unique_ptr<RenderTarget>>>	mpRTV;				// RTVの使用状況とRTVのペア

public:
	DescriptorHeap* GetImGUIDescriptorHeap();
private:
	std::unique_ptr<DescriptorHeap> mpImGUIHeap;
	std::unique_ptr<DescriptorHeap> mpRTVHeap;		// RTV用のデスクリプタヒープ

};

#endif