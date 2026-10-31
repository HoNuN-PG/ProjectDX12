#ifndef __DEBUG_IMGUI_H__
#define __DEBUG_IMGUI_H__

#include <memory>
#include <vector>
#include <Windows.h>
#include <imgui/imgui.h>

#include "Model/MeshBuffer.h"
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Pipeline/PipelineState.h"
#include "System/Rendering/Pipeline/RootSignature.h"
#include "System/Rendering/Texture/RenderTarget.h"

/// <summary>
/// ImGUIの初期化・描画とImGUI::Imageで表示するテクスチャの管理（シングルトン）
/// </summary>
class ImGUIImage
{
private:

	/// <summary>
	/// 1フレームで表示できる画像の最大数
	/// </summary>
	static const int IMAGE_MAX = 32;

public:
	
	static ImGUIImage* GetInstance();
	static void Destroy();	// Uninitしてから破棄する
	
private:
	
	ImGUIImage() = default;
	~ImGUIImage() = default;
	
	static ImGUIImage*  instance;

public:

	/// <summary>
	/// ImGUIの初期化
	/// </summary>
	/// <returns>失敗したらfalse</returns>
	bool Init(HWND hwnd);

	/// <summary>
	/// ImGUIの終了
	/// GPUの処理完了後、UninitDirectXより前に呼ぶ
	/// </summary>
	void Uninit();

	/// <summary>
	/// フレーム開始
	/// </summary>
	void NewFrame();

	/// <summary>
	/// ImGUIの描画コマンドを発行
	/// </summary>
	void Render();

public:

	/// <summary>
	/// ImGUI::Imageで表示するリソースの取得
	/// </summary>
	/// <param name="heap">srvに使用されているヒープ</param>
	/// <param name="srv">表示するテクスチャ</param>
	/// <returns>表示用テクスチャ（1フレームの上限を超えたら0）</returns>
	ImTextureID GetImage(DescriptorHeap* heap, RenderTarget* srv);

	/// <summary>
	/// ImGUI::Imageで表示するリソースの取得
	/// </summary>
	/// <param name="heap">srvに使用されているヒープ</param>
	/// <param name="srv">表示するテクスチャのSRV</param>
	/// <returns>表示用テクスチャ（1フレームの上限を超えたら0）</returns>
	ImTextureID GetImage(DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE srv);

private:

	std::unique_ptr<DescriptorHeap> pHeap;				// フォントと表示画像のSRV
	std::unique_ptr<DescriptorHeap> pRTVHeap;			// 表示画像のRTV
	std::unique_ptr<MeshBuffer> pScreen;
	std::unique_ptr<RootSignature> pRootSignatureData;
	std::unique_ptr<PipelineState> pPipelineData;
	std::vector<std::unique_ptr<RenderTarget>> Images;	// 表示画像（必要になった分だけ作成）
	UINT UsedImageCount = 0;							// 今フレームで使用した表示画像の数

};

#endif
