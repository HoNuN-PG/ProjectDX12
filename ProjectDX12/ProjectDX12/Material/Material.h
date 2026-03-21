#ifndef ___MATERIAL_H___
#define ___MATERIAL_H___

#include <DirectXMath.h>
#include <memory>
#include <string>
#include <vector>

#include "System/Rendering/ConstantBuffer/ConstantBuffer.h"
#include "System/Rendering/Pass/RenderingPass.h"
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Pipeline/PipelineState.h"
#include "System/Rendering/Pipeline/RootSignature.h"
#include "System/Rendering/Texture/Texture.h"

class Material
{
public:
	/// <summary>
	/// 描画タイミング
	/// </summary>
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
	/// <summary>
	/// 初期化デスク
	/// </summary>
	struct Description
	{
		DescriptorHeap* pHeap = nullptr;
		D3D12_CULL_MODE CullMode = D3D12_CULL_MODE_BACK;
		BOOL WriteDepth = TRUE;
		RenderingTiming Timing = RenderingTiming::Forward;
		RenderingPass::RenderingPassType PassType = RenderingPass::RenderingPassType::MAX_RENDERING_PASS_TYPE;
	};

	/// <summary>
	/// マテリアル共通パラメータ
	/// </summary>
	struct CommonParam
	{
		float AlphaCut;
		DirectX::XMFLOAT3 pad1;
	};

public:
	Material();
	virtual ~Material() {};

	// =============================================
	// 初期化
public:
	static void Initialize(std::shared_ptr<Material> material, Description desc);
protected:
	virtual void Initialize(Description desc) = 0;

	// =============================================
	// セットアップ
protected:
	void SetUp(
		DescriptorHeap* heap,
		RootSignature::Description rootsignature,
		PipelineState::Description pipeline,
		UINT rtvNum = 0
	);

	// =============================================
	// バインド
public:
	virtual void Bind(UINT materialinstance) = 0;
protected:
	void BindBase(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT handleNum);
	void BindBase(RootSignature::BindSetting* setting, UINT handleNum);

	// =============================================
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

	// =============================================
	// マテリアルパラメータ
public:
	/// <summary>
	/// テクスチャ追加
	/// </summary>
	/// <param name="path"></param>
	void AddTexture(const char* path);

protected:
	std::vector<std::unique_ptr<Texture>> Textures;

public:
	/// <summary>
	/// WVPの書き込み
	/// マテリアルインスタンスに書き込まれる
	/// </summary>
	/// <param name="data"></param>
	void WriteWVP(void* data, UINT instance);
	void WriteParam(void* data, UINT idx);
	void WriteParams(UINT range, UINT startIdx, D3D12_CPU_DESCRIPTOR_HANDLE startHandle, D3D12_DESCRIPTOR_HEAP_TYPE type);

protected:
	std::vector<std::unique_ptr<ConstantBuffer>> WVP;
	std::vector<std::unique_ptr<ConstantBuffer>> Params;

public:
	RenderingTiming GetRenderTiming() { return Timing; };
	RenderingPass::RenderingPassType GetPassType() { return PassType; }

protected:
	RenderingTiming	Timing;
	RenderingPass::RenderingPassType PassType;

protected:
	DescriptorHeap*	pHeap;
	std::shared_ptr<DescriptorHeap>	pRTVHeap;
	std::unique_ptr<RootSignature> pRootSignatureData;
	std::unique_ptr<PipelineState> pPipelineData;

	// =============================================
	// メッシュシェーダー処理
public:
	virtual int GetMeshShaderSRVStartSlot() const { return -1; }
	virtual int GetAmpShaderSRVStartSlot() const { return -1; }
	virtual void WriteMeshletCount(int count) {}

public:
	
	std::weak_ptr<class RenderingEngine> GetRenderingEngine();

};

#endif