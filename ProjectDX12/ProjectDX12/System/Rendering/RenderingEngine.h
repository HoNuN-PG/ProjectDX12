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

class GameObject;

class RenderingEngine
{
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

	DescriptorHeap* GeStagingHeap() { return pStagingHeap.get(); }
	DescriptorHeap* GetHeap() { return pHeap.get(); }
	DescriptorHeap* GetRTVHeap() { return pRTVHeap.get(); }

private:

	std::shared_ptr<DescriptorHeap> pStagingHeap;
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
	DescriptorHeap::Handle GetGlobalConstantBufferResource(UINT key);
	void WriteGlobalConstantBufferResource(UINT key, void* data);
	// グローバルテクスチャ
	std::shared_ptr<RenderTarget> GetGlobalStagingRenderTarget(UINT key);
	std::shared_ptr<RenderTarget> GetGlobalRenderTarget(UINT key);
	DescriptorHeap::Handle GetGlobalTextureStagingRTV(UINT key);
	DescriptorHeap::Handle GetGlobalTextureRTV(UINT key);
	DescriptorHeap::Handle GetGlobalTextureStagingSRV(UINT key);
	DescriptorHeap::Handle GetGlobalTextureSRV(UINT key);
	void GlobalTextureStagingRTV2SRV(UINT key);
	void GlobalTextureStagingSRV2RTV(UINT key);
	void GlobalTextureRTV2SRV(UINT key);
	void GlobalTextureSRV2RTV(UINT key);
	void CopyGlobalTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT key);

private:

	std::unordered_map<UINT, std::shared_ptr<ConstantBuffer>> GlobalConstantBuffer;
	std::unordered_map<UINT, std::shared_ptr<RenderTarget>> GlobalTextureStaging;
	std::unordered_map<UINT, std::shared_ptr<RenderTarget>> GlobalTexture;
	std::unordered_map<UINT, DXGI_FORMAT> GlobalTextureFormat;

	// ====================
	// DefferedRendering
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

	/// <summary>
	/// GBuffer
	/// </summary>
	enum GBuffer
	{
		Albedo = 0,	// アルベド

		MAX_GBUFFER
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
		RenderingPasses[timing][passType]->Init(pRTVHeap, pStagingHeap, pHeap,pDSVHeap);
	}

	template<typename T>
	T* GetRenderingPass(UINT timing, UINT passType)
	{
		if (RenderingPasses[timing].contains(passType))
		{
			return static_cast<T*>(RenderingPasses[timing][passType].get());
		}
		return nullptr;
	}

	std::shared_ptr<class ShadowPass> GetShadowMapsPass();

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
	void CopyPassTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT timing, UINT type, UINT idx);

	/// <summary>
	/// パスのテクスチャのフォーマットの取得
	/// </summary>
	/// <param name="timing"></param>
	/// <param name="type"></param>
	/// <param name="idx"></param>
	/// <returns></returns>
	std::vector<DXGI_FORMAT> GetPassFormat(UINT timing, UINT type);

private:

	std::shared_ptr<RenderingPass> ShadowMapsPass;			// シャドウマップパス
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