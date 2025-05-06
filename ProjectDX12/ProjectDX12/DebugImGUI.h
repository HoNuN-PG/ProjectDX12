#ifndef __DEBUG_IMGUI_H__
#define __DEBUG_IMGUI_H__

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <Windows.h>

#include "imgui/imgui.h"

#include "DescriptorHeap.h"
#include "MeshBuffer.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "RootSignature.h"

class ConstantBuffer;

class DebugImGUI
{
public:
	struct Vertex
	{
		float pos[3];
		float uv[2];
	};

private:
	static const int HEAP_NUM = 5;

public:
	DebugImGUI();
	~DebugImGUI();
	static MSG Create(HWND _hwnd);

public:
	/// <summary>
	/// ImGUI::Imageで表示するリソースの取得
	/// </summary>
	/// <param name="_heap">_wvpと_srvに使用されているヒープ</param>
	/// <param name="_wvp"></param>
	/// <param name="_srv"></param>
	/// <returns></returns>
	static ImTextureID GetImGUIImage(DescriptorHeap* _heap, ConstantBuffer* _wvp, RenderTarget* _srv);

public:
	// 描画終了時に呼び出し
	static void CompletedDraw();

private:
	static std::unique_ptr<MeshBuffer>									Screen;
	static std::unique_ptr<RootSignature>								RootSignatureData;
	static std::unique_ptr<Pipeline>									PipelineData;
	static std::vector<std::pair<bool,std::unique_ptr<RenderTarget>>>	RTVs;

public:
	static DescriptorHeap* GetImGUIDescriptorHeap();
private:
	static std::unique_ptr<DescriptorHeap> RTVHeap;
	static std::unique_ptr<DescriptorHeap> ImGUIHeap;

};

#endif