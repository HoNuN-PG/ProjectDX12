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
	struct Vertex
	{
		float pos[3];
		float normal[3];
		float uv[2];
		float color[4];
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
	std::unique_ptr<MeshBuffer>									Screen;
	std::unique_ptr<RootSignature>								RootSignatureData;
	std::unique_ptr<Pipeline>									PipelineData;
	std::vector<std::pair<bool,std::unique_ptr<RenderTarget>>>	RTVs;

public:
	DescriptorHeap* GetImGUIDescriptorHeap();
private:
	std::unique_ptr<DescriptorHeap> ImGUIHeap;
	std::unique_ptr<DescriptorHeap> RTVHeap;

};

#endif