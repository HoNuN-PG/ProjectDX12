#ifndef __DEBUG_IMGUI_H__
#define __DEBUG_IMGUI_H__

#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <Windows.h>
#include <imgui/imgui.h>

#include "Model/MeshBuffer.h"
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Pipeline/PipelineState.h"
#include "System/Rendering/Pipeline/RootSignature.h"
#include "System/Rendering/Texture/RenderTarget.h"

class ConstantBuffer;

class ImGUIImage
{
public:

	struct Vertex
	{
		float pos[3];
		float uv[2];
	};

private:

	/// <summary>
	/// 必要ヒープ数
	/// </summary>
	static const int HEAP_NUM = 32;

public:

	ImGUIImage();
	~ImGUIImage();

public:

	static MSG Create(HWND _hwnd);

public:

	/// <summary>
	/// ImGUI::Imageで表示するリソースの取得
	/// </summary>
	/// <param name="_heap">_srvに使用されているヒープ</param>
	/// <param name="_srv"></param>
	/// <returns></returns>
	static ImTextureID GetImage(DescriptorHeap* _heap, RenderTarget* _srv);

public:

	/// <summary>
	/// 描画終了
	/// </summary>
	static void Completed();

private:

	static std::unique_ptr<MeshBuffer> pScreen;
	static std::unique_ptr<RootSignature> pRootSignatureData;
	static std::unique_ptr<PipelineState> pPipelineData;
	static std::vector<std::pair<bool,std::unique_ptr<RenderTarget>>> Images;

public:

	static DescriptorHeap* GetImGUIDescriptorHeap() { return pHeap.get(); }

private:

	static std::unique_ptr<DescriptorHeap> pHeap;
	static std::unique_ptr<DescriptorHeap> pRTVHeap;

};

#endif