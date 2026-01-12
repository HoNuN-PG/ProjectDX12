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

// 汎用パラメータ
struct CommonParam
{
	float AlphaCut;
	DirectX::XMFLOAT3 pad1;
};

class Material
{
	// 描画タイミング
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
	// 初期化デスク
	struct Description
	{
		DescriptorHeap* pHeap = nullptr;
		D3D12_CULL_MODE CullMode = D3D12_CULL_MODE_BACK;
		BOOL WriteDepth = TRUE;
		RenderingTiming Timing = RenderingTiming::Forward;
		RenderingPass::RenderingPassType PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;
	};

public:
	Material();
	virtual ~Material() {};

	// 初期化
public:
	static void Initialize(std::shared_ptr<Material> material, Description desc);
protected:
	virtual void Initialize(Description desc) = 0;

	// セットアップ
protected:
	void SetUp(
		DescriptorHeap* heap,
		RootSignature::Description rootsignature,
		Pipeline::Description pipeline,
		UINT rtvNum = 0
	);

	// 設定
public:
	virtual void Bind(UINT materialinstance) = 0;
protected:
	void BindBase(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT handleNum);
	void BindBase(RootSignature::CustomBindSetting* setting, UINT handleNum);

	// マテリアルインスタンス
public:
	/// <summary>
	/// マテリアルインスタンスの追加
	/// </summary>
	UINT AddMaterialInstance();

	/// <summary>
	/// マテリアルインスタンスの削除
	/// </summary>
	/// <param name="instance"></param>
	void RemoveMaterialInstance(UINT instance);

protected:
	unsigned int		MaterialInstanceCount;	// マテリアルインスタンスの総数
	std::vector<bool>	MaterialInstanceList;	// マテリアルインスタンスの使用状況

public:
	/// <summary>
	/// テクスチャ追加
	/// </summary>
	/// <param name="path"></param>
	void AddTexture(const char* path);

public:
	/// <summary>
	/// WVPの書き込み
	/// マテリアルインスタンスに書き込まれる
	/// </summary>
	/// <param name="data"></param>
	void WriteWVP(void* data, UINT instance);
	void WriteParam(void* data, UINT idx);
	void WriteParams(UINT range, UINT startIdx, D3D12_CPU_DESCRIPTOR_HANDLE startHandle, D3D12_DESCRIPTOR_HEAP_TYPE type);

public:
	virtual int GetMeshShaderSRVStartSlot() const { return -1; }
	virtual int GetAmpShaderSRVStartSlot() const { return -1; }
	virtual void WriteMeshletCount(int count) {}

public:
	RenderingTiming GetRenderTiming() { return Timing; };
	RenderingPass::RenderingPassType GetPassType() { return PassType; }

	// マテリアルパラメータ
protected:
	RenderingTiming									Timing;
	RenderingPass::RenderingPassType				PassType;

	std::vector<std::unique_ptr<ConstantBuffer>>	WVP;
	std::vector<std::unique_ptr<ConstantBuffer>>	Params;

	std::vector<std::unique_ptr<Texture>>			Textures;

protected:
	DescriptorHeap*									pHeap;
	std::shared_ptr<DescriptorHeap>					pRTVHeap;
	std::unique_ptr<RootSignature>					RootSignatureData;
	std::unique_ptr<Pipeline>						PipelineData;

};

#endif