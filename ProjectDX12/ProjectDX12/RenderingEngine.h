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
#include "PostProcess.h"
#include "RenderingPass.h"

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

	// GBuffer
private:
	enum GBuffer
	{
		Albedo = 0,		// アルベド
		Normal,			// 法線
		
		MAX_GBUFFER
	};

	// グローバルリソース
public:
	// グローバル定数バッファ
	static DescriptorHeap::Handle GetGlobalConstantBufferResource(UINT key);
	// グローバルテクスチャ
	static std::shared_ptr<RenderTarget> GetGlobalRenderTarget(UINT key);
	static DescriptorHeap::Handle GetGlobalTextureRTV(UINT key);
	static DescriptorHeap::Handle GetGlobalTextureSRV(UINT key);
	// リソース切り替え
	static void GlobalTextureRTV2SRV(UINT key);
	static void GlobalTextureSRV2RTV(UINT key);
private:
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
private:
	std::unordered_map<UINT, std::unique_ptr<RenderingPass>> RenderingPasses;

	// レンダリングオブジェクト
public:
	// 描画オブジェクトの追加
	void AddRenderObject(GameObject& obj, Material::RenderingTiming timing);
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
	std::vector<RenderingInfo> DefferedObjects;				// ディファードライティングオブジェクト
	std::vector<RenderingInfo> ForwardObjects;				// フォワードライティングオブジェクト
	std::unique_ptr<PostProcess> ObjectPostProcess;			// オブジェクト描画後のポストプロセス
	std::unique_ptr<PostProcess> CanvasPostProcess;			// キャンバス描画後のポストプロセス
	std::list<std::weak_ptr<Material>> RenderingMaterials;				// 作成されたマテリアル群

	// レンダリング
public:
	static Material::RenderingTiming GetCurrentRenderingPass() { return CurrentRenderingTiming; }
private:
	void OpaqueDepthNormalRendering();
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