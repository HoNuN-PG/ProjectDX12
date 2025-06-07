#ifndef ___RENDERING_ENGINE_H___
#define ___RENDERING_ENGINE_H___

#include <vector>
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
	static DescriptorHeap::Handle GetGlobalConstantBufferResource(UINT key);
	static std::shared_ptr<RenderTarget> GetGlobalRenderTarget(UINT key);
	static DescriptorHeap::Handle GetGlobalTextureRTV(UINT key);
	static DescriptorHeap::Handle GetGlobalTextureSRV(UINT key);
	static void GlobalTextureRTV2SRV(UINT key);
	static void GlobalTextureSRV2RTV(UINT key);
private:
	static std::unordered_map<UINT, std::shared_ptr<ConstantBuffer>> GlobalConstantBuffer;
	static std::unordered_map<UINT, std::shared_ptr<RenderTarget>> GlobalTexture;
private:
	void WriteGlobalConstantBufferResource();
private:
	CameraDebug* Camera;
	LightBase* Light;

	// リソース
private:
	std::unique_ptr<DepthStencil> DSV;

	// レンダリングパス
private:
	std::unordered_map<UINT, std::unique_ptr<RenderingPass>> RenderingPasses;

	// レンダリングオブジェクト
public:
	void AddRenderObject(GameObject& obj, RenderingPass::RenderingPassType pass ,Material::MainPassRenderingTiming timing);
	// ポストプロセスの追加や取得
	template <typename T>
	T* AddVolume()
	{
		return ObjectPostProcess->AddVolume<T>();
	}
	template <typename T>
	T* GetVolume()
	{
		return ObjectPostProcess->GetVolume<T>();
	}
	template <typename T>
	T* AddCanvasVolume()
	{
		return CanvasPostProcess->AddVolume<T>();
	}
	template <typename T>
	T* GetCanvasVolume()
	{
		return CanvasPostProcess->GetVolume<T>();
	}
private:
	std::vector<std::vector<RenderingInfo>> RenderObjects;
	std::unique_ptr<PostProcess> ObjectPostProcess;			// オブジェクト描画後のポストプロセス
	std::unique_ptr<PostProcess> CanvasPostProcess;			// キャンバス描画後のポストプロセス

	// レンダリング
public:
	static RenderingPass::RenderingPassType GetCurrentRenderingPass() { return CurrentRenderingPass; }
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
private:
	static RenderingPass::RenderingPassType CurrentRenderingPass;

};

#endif