#ifndef ___RENDERING_ENGINE_H___
#define ___RENDERING_ENGINE_H___

#include <vector>
#include <list>
#include <memory>
#include <unordered_map>

#include "DescriptorHeap.h"
#include "DepthStencil.h"
#include "RenderTarget.h"
#include "ConstantBuffer.h"

#include "Material.h"
#include "M_Shadow.h"

#include "RenderingPass.h"

#include "PostProcess.h"

class GameObject;
class CameraBase;
class LightBase;

class RenderingEngine
{
public:
	// 描画パス群
	using PASSES = std::unordered_map<UINT,  // パスの種類
		std::unique_ptr<RenderingPass>>;

	// RenderingInfo
public:
	struct RenderingInfo
	{
		GameObject& obj;
	};

	// GBuffer
private:
	enum GBuffer
	{
		Albedo = 0,		// アルベド
		Normal,			// 法線

		MAX_GBUFFER
	};

public:
	RenderingEngine() {};
	~RenderingEngine() {};
	void Init();
	void Uninit();
	void Update();
	void Draw();

	// Utillity
public:
	static void CopyTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE src, D3D12_CPU_DESCRIPTOR_HANDLE dest);

	// 描画ヒープ
public:
	DescriptorHeap* GetRenderingHeap() { return RenderingHeap.get(); }
	DescriptorHeap* GetRTVHeap() { return RTVHeap.get(); }
private:
	std::shared_ptr<DescriptorHeap> RenderingHeap;
	std::shared_ptr<DescriptorHeap>	RTVHeap;
	std::shared_ptr<DescriptorHeap>	DSVHeap;
	// 描画リソース
public:
	std::shared_ptr<DepthStencil> GetDSV() { return DSV; };
private:
	std::shared_ptr<DepthStencil> DSV;

	// グローバルリソース
public:
	// グローバル定数バッファ
	DescriptorHeap::Handle GetGlobalConstantBufferResource(UINT key);
	void WriteGlobalConstantBufferResource(UINT key, void* data);
	// グローバルテクスチャ
	std::shared_ptr<RenderTarget> GetGlobalRenderTarget(UINT key);
	DescriptorHeap::Handle GetGlobalTextureRTV(UINT key);
	DescriptorHeap::Handle GetGlobalTextureSRV(UINT key);
	void CopyGlobalTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT key);
	void GlobalTextureRTV2SRV(UINT key);
	void GlobalTextureSRV2RTV(UINT key);
private:
	std::unordered_map<UINT, std::shared_ptr<ConstantBuffer>> GlobalConstantBuffer;
	std::unordered_map<UINT, std::shared_ptr<RenderTarget>> GlobalTexture;

	// 環境
private:
	std::shared_ptr<CameraBase> Camera;
	std::shared_ptr<LightBase> Light;

	// レンダリングパス
public:
	template<typename T>
	void AddRenderingPass(UINT timing,UINT passType)
	{
		if (RenderingPasses[timing].contains(passType)) 
			return;
		RenderingPasses[timing][passType] = std::make_unique<T>();
		RenderingPasses[timing][passType]->Init(RTVHeap,RenderingHeap,DSVHeap);
	}
	std::shared_ptr<RenderTarget> GetPassTexture(UINT timing, UINT type, UINT idx);
	void CopyPassTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT timing, UINT type, UINT idx);
private:
	std::unique_ptr<RenderingPass> ShadowMapsPass;			// シャドウマップパス
	std::unique_ptr<RenderingPass> ODepthNormalPass;		// 不透明深度法線パス
	std::unordered_map<UINT,								// 描画タイミング
		PASSES> RenderingPasses;							// 描画パス群

	// レンダリングオブジェクト
public:
	// 描画オブジェクトの追加
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
	// 作成したマテリアルの追加
	void AddRenderingMaterial(std::shared_ptr<Material> material);
private:
	std::vector<RenderingInfo> EnvironmentObjects;			// 環境描画オブジェクト
	std::vector<RenderingInfo> DefferedObjects;				// ディファードライティングオブジェクト
	std::vector<RenderingInfo> ForwardObjects;				// フォワードライティングオブジェクト
	std::unique_ptr<PostProcess> ObjectPostProcess;			// オブジェクト描画後のポストプロセス
	std::unique_ptr<PostProcess> CanvasPostProcess;			// キャンバス描画後のポストプロセス
	std::list<std::weak_ptr<Material>> RenderingMaterials;	// 作成されたマテリアル群

	// レンダリング関数
public:
	Material::RenderingTiming GetCurrentRenderingPass() { return CurrentRenderingTiming; }
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
	void RefreshRendering();

};

#endif