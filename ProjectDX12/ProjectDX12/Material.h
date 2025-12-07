#ifndef ___MATERIAL_H___
#define ___MATERIAL_H___

#include <DirectXMath.h>
#include <memory>
#include <string>
#include <vector>

// System/Constant
#include "ConstantBuffer.h"
// System/Rendering/Pass
#include "RenderingPass.h"
// System/Rendering/Pipeline
#include "DescriptorHeap.h"
#include "Pipeline.h"
#include "RootSignature.h"
// System/Rendering/Texture
#include "Texture.h"

struct CommonParam
{
	float AlphaCut;
	DirectX::XMFLOAT3 pad1;
};

class Material
{
public:
	enum RenderingTiming
	{
		Shadow = 0,					// シャドウ
		OpaqueDepthNormal,			// 不透明深度法線
		AfterOpaqueDepthNormal,		// 不透明深度法線描画後
		Environment,				// 環境（スカイボックス等）
		Deffered,					// ディファード
		Forward,					// フォワード
		TranslucentDepthNormal,		// 透明深度法線
		Canvas,						// キャンバス
		Other,						// その他

		MAX_RENDERING_PASS_TYPE
	};

public:
	Material();
	virtual ~Material() {};

public:
	struct Description
	{
		D3D12_CULL_MODE cull = D3D12_CULL_MODE_BACK;
		BOOL			WriteDepth = TRUE;
	};

public:
	// マテリアル全体初期化
	static void Initialize(
		std::shared_ptr<Material> material, // 初期化するマテリアル
		DescriptorHeap* heap,				// 使用ヒープ
		Description desc,					// 初期化デスク
		RenderingTiming timing = RenderingTiming::Forward,
		RenderingPass::RenderingPassType passType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE
	);
protected:
	// マテリアル個別初期化
	virtual void Initialize(DescriptorHeap* heap, Description desc) = 0;

protected:
	// セットアップ
	void SetUp(
		DescriptorHeap* heap,
		RootSignature::DescriptionTable rootsignature,
		Pipeline::Description pipeline,
		UINT rtvNum = 0
	);

public:
	/// <summary>
	/// マテリアル個別設定
	/// 設定後MaterialInstanceIdxが更新される
	/// </summary>
	virtual void Bind() = 0;
protected:
	// マテリアル全体設定
	void BindBase(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT handleNum);

public:
	/// <summary>
	/// マテリアルインスタンスの追加
	/// <return>マテリアルインスタンスUID</return>
	/// </summary>
	void AddMaterialInstance();

	/// <summary>
	/// テクスチャ追加
	/// </summary>
	/// <param name="path"></param>
	void AddTexture(const char* path);

	/// <summary>
	/// MaterialInstanceIdxにWVPを書き込み
	/// WVPの書き込み直後にBindすることでMaterialInstanceIdxに書き込まれたWVPで設定を行う
	/// </summary>
	/// <param name="data"></param>
	void WriteWVP(void* data);
	void WriteParam(void* data, UINT idx);
	void WriteParams(UINT range, UINT startIdx, D3D12_CPU_DESCRIPTOR_HANDLE startHandle, D3D12_DESCRIPTOR_HEAP_TYPE type);

	/// <summary>
	/// 描画終了時に呼び出し
	/// </summary>
	void RefreshRendering();

public:
	RenderingTiming GetRenderTiming() { return Timing; };
	RenderingPass::RenderingPassType GetPassType() { return PassType; }

protected:
	// マテリアルパラメータ
	RenderingTiming									Timing;
	RenderingPass::RenderingPassType				PassType;

	DescriptorHeap*									Heap;
	std::shared_ptr<DescriptorHeap>					RTVHeap;
	std::unique_ptr<RootSignature>					RootSignatureData;
	std::unique_ptr<Pipeline>						PipelineData;

	std::vector<std::unique_ptr<Texture>>			Textures;

	// マテリアルインスタンス情報
	unsigned int									MaterialInstanceCount;				// マテリアルインスタンスの総数
	unsigned int									MaterialInstanceIdx;				// マテリアルインスタンスのインデックス
																						// １つのマテリアルで複数回オブジェクトを描画する際は
																						// マテリアルインスタンスを使い切ってから描画する

	std::vector<std::unique_ptr<ConstantBuffer>>	WVP;
	std::vector<std::unique_ptr<ConstantBuffer>>	Params;

};

#endif