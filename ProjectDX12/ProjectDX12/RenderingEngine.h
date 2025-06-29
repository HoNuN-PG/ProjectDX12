#ifndef ___RENDERING_ENGINE_H___
#define ___RENDERING_ENGINE_H___

#include <vector>
#include <list>
#include <memory>
#include <unordered_map>

#include "ConstantBuffer.h"
#include "DescriptorHeap.h"

#include "DepthStencil.h"
#include "RenderTarget.h"

#include "Material.h"
#include "RenderingPass.h"
#include "PostProcess.h"

class GameObject;
class CameraDebug;
class LightBase;

class RenderingEngine
{
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

	// グローバルヒープ
public:
	static std::shared_ptr<DescriptorHeap> GetGlobalHeap()
	{ return GlobalHeap; }
private:
	static std::shared_ptr<DescriptorHeap> GlobalHeap;

	// ヒープ
public:
	std::shared_ptr<DescriptorHeap> GetHeap()
	{ return Heap; }
private:
	std::shared_ptr<DescriptorHeap>	Heap;
	std::shared_ptr<DescriptorHeap>	RTVHeap;
	std::shared_ptr<DescriptorHeap>	DSVHeap;

	// グローバルリソース
public:
	// グローバル定数バッファ
	static DescriptorHeap::Handle GetGlobalConstantBufferResource(UINT key);
	// グローバルテクスチャ
	static std::shared_ptr<RenderTarget> GetGlobalRenderTarget(UINT key);
	static DescriptorHeap::Handle GetGlobalTextureRTV(UINT key);
	static DescriptorHeap::Handle GetGlobalTextureSRV(UINT key);
	// テクスチャリソース切り替え
	static void GlobalTextureRTV2SRV(UINT key);
	static void GlobalTextureSRV2RTV(UINT key);
private:
	// グローバルリソース
	static std::unordered_map<UINT, std::shared_ptr<ConstantBuffer>> GlobalConstantBuffer;
	static std::unordered_map<UINT, std::shared_ptr<RenderTarget>> GlobalTexture;

	// グローバルリソースオブジェクト
private:
	void WriteGlobalConstantBufferResource();
private:
	std::shared_ptr<CameraDebug> Camera;
	std::shared_ptr<LightBase> Light;

	// リソース
private:
	std::unique_ptr<DepthStencil> DSV;

	// レンダリングパス
public:
	template<typename T>
	void AddRenderingPass(UINT timing,UINT passType)
	{
		if (!RenderingPasses.contains(timing))
			RenderingPasses[timing] = std::unordered_map<UINT, std::unique_ptr<RenderingPass>>();
		if (RenderingPasses[timing].contains(passType)) 
			return;
		RenderingPasses[timing][passType] = std::make_unique<T>();
	}
	std::shared_ptr<RenderTarget> GetPassTexture(UINT timing, UINT type, UINT idx);
private:
	std::unique_ptr<RenderingPass> ODepthNormalPass;	// 不透明深度法線パス
	std::unordered_map<UINT,std::unordered_map<UINT, std::unique_ptr<RenderingPass>>> 
		RenderingPasses;								// 描画タイミングとパスの種類をキーとしたパス群

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

	// 描画リソース
private:
	std::vector<RenderingInfo> DefferedObjects;				// ディファードライティングオブジェクト
	std::vector<RenderingInfo> ForwardObjects;				// フォワードライティングオブジェクト
	std::unique_ptr<PostProcess> ObjectPostProcess;			// オブジェクト描画後のポストプロセス
	std::unique_ptr<PostProcess> CanvasPostProcess;			// キャンバス描画後のポストプロセス
	std::list<std::weak_ptr<Material>> RenderingMaterials;	// 作成されたマテリアル群

	// レンダリング
public:
	static Material::RenderingTiming GetCurrentRenderingPass() { return CurrentRenderingTiming; }
private:
	void OpaqueDepthNormalRendering();
	void AfterOpaqueDepthNormalRendering();
	void DefferedRendering();
	void DefferedLighting();
	void ForwardRendering();
	void TranslucentDepthNormalRendering();
	void ObjectPostProcessRendering();
	void CanvasPostProcessRendering();
	void ViewDepthNormal();
	void ViewGBuffers();
	void EndRendering();
private:
	static Material::RenderingTiming CurrentRenderingTiming;

};

#endif