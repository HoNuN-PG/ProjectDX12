#ifndef ___RENDERING_ENGINE_H___
#define ___RENDERING_ENGINE_H___

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

// Material
#include "Material.h"
// Material/Materials
#include "M_Shadow.h"

// System/Constant
#include "ConstantBuffer.h"
// System/Rendering/Pass
#include "RenderingPass.h"
// System/Rendering/Pipeline
#include "DescriptorHeap.h"
// System/Rendering/Texture
#include "DepthStencil.h"
#include "RenderTarget.h"
// System/Rendering
#include "PostProcess.h"

class CameraBase;
class LightBase;

class GameObject;

/// <summary>
/// ディファードデータ
/// </summary>
struct DefferedData
{
	std::unique_ptr<RootSignature>					RootSignatureData;
	std::unique_ptr<Pipeline>						PipelineData;
	std::vector<std::unique_ptr<ConstantBuffer>>	Params;
};

class RenderingEngine
{
public:


	// ====================
	// 描画情報
public:
	// 描画するゲームオブジェクト
	struct RenderingInfo
	{
		GameObject& obj;
	};

	// ====================
	// GBuffer
private:
	enum GBuffer
	{
		Albedo = 0,	// アルベド

		MAX_GBUFFER
	};

public:
	RenderingEngine() {};
	~RenderingEngine() {};
	void Init();
	void Uninit();
	void Update();
	void Draw();

	// ====================
	// Utillity
public:
	static void CopyTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE src, D3D12_CPU_DESCRIPTOR_HANDLE dest);

	// ====================
	// 描画ヒープ
public:
	DescriptorHeap* GetHeap() { return pHeap.get(); }
	DescriptorHeap* GetRTVHeap() { return pRTVHeap.get(); }
private:
	std::shared_ptr<DescriptorHeap> pHeap;
	std::shared_ptr<DescriptorHeap>	pRTVHeap;
	std::shared_ptr<DescriptorHeap>	pDSVHeap;

	// ====================
	// 描画リソース
public:
	std::shared_ptr<DepthStencil> GetDSV() { return DSV; };
private:
	std::shared_ptr<DepthStencil> DSV;

	// ====================
	// グローバルリソース
public:
	// グローバル定数バッファ
	void WriteGlobalConstantBufferResource(UINT key, void* data);
	DescriptorHeap::Handle GetGlobalConstantBufferResource(UINT key);
	// グローバルテクスチャ
	void CopyGlobalTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT key);
	std::shared_ptr<RenderTarget> GetGlobalRenderTarget(UINT key);
	DescriptorHeap::Handle GetGlobalTextureRTV(UINT key);
	DescriptorHeap::Handle GetGlobalTextureSRV(UINT key);
	void GlobalTextureRTV2SRV(UINT key);
	void GlobalTextureSRV2RTV(UINT key);
private:
	std::unordered_map<UINT, std::shared_ptr<ConstantBuffer>> GlobalConstantBuffer;
	std::unordered_map<UINT, std::shared_ptr<RenderTarget>> GlobalTexture;

	// ====================
	// 環境オブジェクト
public:
	std::shared_ptr<CameraBase> GetCamera() { return Camera; }
private:
	std::shared_ptr<CameraBase> Camera;
	std::shared_ptr<LightBase> Light;

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
		RenderingPasses[timing][passType] = std::make_unique<T>();
		RenderingPasses[timing][passType]->Init(pRTVHeap,pHeap,pDSVHeap);
	}
	std::shared_ptr<class ShadowPass> GetShadowMapsPass();
	template<typename T>
	T* GetRenderingPass(UINT timing, UINT passType)
	{
		if (RenderingPasses[timing].contains(passType))
		{
			return static_cast<T*>(RenderingPasses[timing][passType].get());
		}
		return nullptr;
	}

	/// <summary>
	/// パスのテクスチャのコピー
	/// </summary>
	/// <param name="dest">コピー先</param>
	/// <param name="timing">描画タイミング</param>
	/// <param name="type">パスの種類</param>
	/// <param name="idx">パス内のテクスチャインデックス</param>
	void CopyPassTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT timing, UINT type, UINT idx);

	/// <summary>
	/// パスのテクスチャの取得
	/// </summary>
	/// <param name="timing">描画タイミング</param>
	/// <param name="type">パスの種類</param>
	/// <param name="idx">パス内のテクスチャインデックス</param>
	/// <returns></returns>
	std::shared_ptr<RenderTarget> GetPassTexture(UINT timing, UINT type, UINT idx);

private:
	std::shared_ptr<RenderingPass> ShadowMapsPass;			// シャドウマップパス
	std::unique_ptr<RenderingPass> ODepthNormalPass;		// 不透明深度法線パス
	std::unordered_map<UINT, PASSES> RenderingPasses;		// key=描画タイミング,value=描画パス群

	// ====================
	// レンダリングオブジェクト
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
	// DefferedShader
private:
	void SetupDefferedShader();
private:
	DefferedData DefferedLightingShader;

	// ====================
	// コンポーネントやマテリアルの参照
public:
	// 作成した描画コンポーネントの参照を追加
	void RegisterRenderingComponentRef(std::shared_ptr<class RenderingComponent> component);
	// 作成したマテリアルの参照を追加
	void RegisterMaterialRef(std::shared_ptr<Material> material);
private:
	std::list<std::weak_ptr<class RenderingComponent>> RenderingComponents;	// 作成された描画コンポーネントの参照
	std::list<std::weak_ptr<Material>> RenderingMaterials;					// 作成されたマテリアルの参照

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