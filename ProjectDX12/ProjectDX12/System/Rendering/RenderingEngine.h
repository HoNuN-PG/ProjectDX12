#ifndef ___RENDERING_ENGINE_H___
#define ___RENDERING_ENGINE_H___

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Material/Material.h"
#include "Material/Materials/M_Shadow.h"

#include "System/Rendering/ConstantBuffer/ConstantBuffer.h"
#include "System/Rendering/Pass/RenderingPass.h"
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Texture/DepthStencil.h"
#include "System/Rendering/Texture/RenderTarget.h"
#include "System/Rendering/Volume/PostProcess.h"

class CameraBase;
class LightBase;
class Copy;
class Gauss;

class GameObject;

class RenderingEngine
{
public:

	RenderingEngine();
	~RenderingEngine();
	void Init();
	void Uninit();
	void Update();
	void Draw();

	// ====================
	// Utillity
public:

	static void CopyTexture(D3D12_CPU_DESCRIPTOR_HANDLE src, D3D12_CPU_DESCRIPTOR_HANDLE dest);
	Copy* GetCopy() { return pCopy.get(); }
	Gauss* GetGauss() { return pGauss.get(); }

private:

	std::unique_ptr<Copy>	pCopy;	// テクスチャのコピー処理
	std::unique_ptr<Gauss>	pGauss;	// ガウスぼかし処理（Copyクラスを使用する）

	// ====================
	// 描画ヒープ
public:

	DescriptorHeap* GeStagingHeap() { return pStagingHeap.get(); }
	DescriptorHeap* GetHeap() { return pHeap.get(); }
	DescriptorHeap* GetRTVHeap() { return pRTVHeap.get(); }

private:

	std::unique_ptr<DescriptorHeap> pStagingHeap;
	std::unique_ptr<DescriptorHeap> pHeap;
	std::unique_ptr<DescriptorHeap>	pRTVHeap;
	std::unique_ptr<DescriptorHeap>	pDSVHeap;

	// ディスクリプタヒープの最大数
	static constexpr UINT STAGING_HEAP_SIZE	= 256;
	static constexpr UINT HEAP_SIZE			= 256;
	static constexpr UINT RTV_HEAP_SIZE		= 64;
	static constexpr UINT DSV_HEAP_SIZE		= 32;

	// ====================
	// グローバルリソース
public:

	// グローバル定数バッファ
	DescriptorHeap::Handle GetGlobalConstantBufferResource(UINT key);
	void WriteGlobalConstantBufferResource(UINT key, void* data);
	// グローバルテクスチャ
	RenderTarget* GetGlobalStagingRenderTarget(UINT key);
	void CopyGlobalTexture(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT key);

private:

	std::unordered_map<UINT, std::unique_ptr<ConstantBuffer>> GlobalConstantBuffer;
	std::unordered_map<UINT, std::unique_ptr<RenderTarget>> GlobalTextureStaging;
	std::unordered_map<UINT, DescriptorHeap::Handle> GlobalTextureSRV;	// シェーダーから参照するSRV（ステージングのSRVをコピーしたもの）
	std::unordered_map<UINT, DXGI_FORMAT> GlobalTextureFormat;

	// ====================
	// リソース
public:

	DepthStencil* GetDSV() { return DSV.get(); }

private:

	std::unique_ptr<DepthStencil> DSV;
	
	// ====================
	// 環境オブジェクト
private:

	enum CameraType
	{
		MAIN = 0,
		FRUSTUM,

		MAX
	};
	CameraType CurrentCameraType = CameraType::MAIN;

public:

	std::shared_ptr<CameraBase> GetCamera() { return Camera[CurrentCameraType]; }

private:

	std::shared_ptr<CameraBase> Camera[CameraType::MAX];
	std::shared_ptr<LightBase> Light;

	// ====================
	// DefferedRendering
private:

	/// <summary>
	/// GBuffer
	/// </summary>
	enum GBuffer
	{
		Albedo = 0,	// アルベド

		MAX_GBUFFER
	};

public:

	/// <summary>
	/// ディファードデータ
	/// </summary>
	struct DefferedData
	{
		std::unique_ptr<RootSignature> pRootSignatureData;
		std::unique_ptr<PipelineState> pPipelineData;
		std::vector<std::unique_ptr<ConstantBuffer>> Params;
	};

private:

	DefferedData DefferedLightingShader;
	
	// ====================
	// レンダリングオブジェクト
public:

	// 描画するゲームオブジェクト
	struct RenderingInfo
	{
		GameObject& obj;
	};

public:

	// オブジェクトの描画登録
	void AddRenderObject(GameObject& obj, UINT timing, UINT passType);
	// ポストプロセスの追加や取得
	template <typename T>
	std::shared_ptr<T> AddVolume()
	{
		return ObjectPostProcess->AddVolume<T>();
	}
	template <typename T>
	std::shared_ptr<T> GetVolume()
	{
		return ObjectPostProcess->GetVolume<T>();
	}
	template <typename T>
	std::shared_ptr<T> AddCanvasVolume()
	{
		return CanvasPostProcess->AddVolume<T>();
	}
	template <typename T>
	std::shared_ptr<T> GetCanvasVolume()
	{
		return CanvasPostProcess->GetVolume<T>();
	}

private:

	std::vector<RenderingInfo> EnvironmentObjects;	// 環境描画オブジェクト
	std::vector<RenderingInfo> DefferedObjects;		// ディファードライティングオブジェクト
	std::vector<RenderingInfo> ForwardObjects;		// フォワードライティングオブジェクト
	std::unique_ptr<PostProcess> ObjectPostProcess;	// オブジェクト描画後のポストプロセス
	std::unique_ptr<PostProcess> CanvasPostProcess;	// キャンバス描画後のポストプロセス

	// ====================
	// レンダリングパス
public:

	// 描画パス群:key=パスの種類,value=パス
	using PASSES = std::unordered_map<UINT, std::unique_ptr<RenderingPass>>;

public:

	template<typename T>
	void AddRenderingPass(UINT timing,UINT passType)
	{
		if (RenderingPasses[timing].contains(passType)) 
			return;
		RenderingPasses[timing][passType] = std::make_unique<T>(*this);
		RenderingPasses[timing][passType]->Init(pRTVHeap.get(), pStagingHeap.get(), pHeap.get(), pDSVHeap.get());
	}

	template<typename T>
	T* GetRenderingPass(UINT timing, UINT passType)
	{
		return static_cast<T*>(FindPass(timing, passType));
	}

	class ShadowPass* GetShadowMapsPass();

	/// <summary>
	/// パスのテクスチャの取得
	/// </summary>
	/// <param name="timing">描画タイミング</param>
	/// <param name="type">パスの種類</param>
	/// <param name="idx">パス内のテクスチャインデックス</param>
	/// <returns></returns>
	std::shared_ptr<RenderTarget> GetPassTextureStaging(UINT timing, UINT type, UINT idx);
	std::shared_ptr<RenderTarget> GetPassTexture(UINT timing, UINT type, UINT idx);

	/// <summary>
	/// パスのテクスチャのコピー
	/// </summary>
	/// <param name="dest">コピー先</param>
	/// <param name="timing">描画タイミング</param>
	/// <param name="type">パスの種類</param>
	/// <param name="idx">パス内のテクスチャインデックス</param>
	void CopyPassTexture(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT timing, UINT type, UINT idx);

	/// <summary>
	/// パスのテクスチャのフォーマットの取得
	/// </summary>
	/// <param name="timing"></param>
	/// <param name="type"></param>
	/// <param name="idx"></param>
	/// <returns></returns>
	std::vector<DXGI_FORMAT> GetPassFormat(UINT timing, UINT type);

private:

	// 描画タイミングとパスの種類からパスを検索（見つからなければnullptr）
	RenderingPass* FindPass(UINT timing, UINT type);

	std::unique_ptr<class ShadowPass> ShadowMapsPass;			// シャドウマップパス
	std::unique_ptr<RenderingPass> ODepthNormalPass;		// 不透明深度法線パス
	std::unordered_map<UINT, PASSES> RenderingPasses;		// key=描画タイミング,value=描画パス群

	// ====================
	// レンダリング関数
public:

	Material::RenderingTiming GetCurrentRenderingTiming() { return CurrentRenderingTiming; }

private:

	Material::RenderingTiming CurrentRenderingTiming;

private:

	void ShadowMapsRendering();
	void OpaqueDepthNormalRendering();
	void AfterOpaqueDepthNormalRendering();
	void EnvironmentRendering();
	void DefferedRendering();
	void DefferedLighting();
	void ForwardRendering();
	void TranslucentDepthNormalRendering();
	void ObjectPostProcessRendering();
	void CanvasPostProcessRendering();
	void ViewShadowMaps();
	void ViewDepthNormal();
	void ViewGBuffers();
	void ViewPasses();

};

#endif