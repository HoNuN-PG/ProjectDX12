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
	static DescriptorHeap* GetGlobalHeap()
	{ return GlobalHeap.get(); }
private:
	static std::unique_ptr<DescriptorHeap> GlobalHeap;
	std::shared_ptr<DescriptorHeap>	GlobalRTVHeap;

	// ヒープ
public:
	DescriptorHeap* GetHeap()
	{ return Heap.get(); }
private:
	std::shared_ptr<DescriptorHeap>	Heap;
	std::unique_ptr<DescriptorHeap>	DSVHeap;

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
	static DescriptorHeap::Handle GetGlobalTextureResource(UINT key);
private:
	static std::unordered_map<UINT, std::unique_ptr<ConstantBuffer>> GlobalConstantBuffer;
	static std::unordered_map<UINT, std::unique_ptr<RenderTarget>> GlobalTexture;
private:
	void WriteGlobalConstantBufferResource();
private:
	CameraDebug* Camera;
	LightBase* Light;

	// リソース
private:
	std::unique_ptr<DepthStencil> DSV;

	// レンダリングオブジェクトの追加
public:
	void AddRenderObject(GameObject& obj, int timing);
private:
	std::vector<std::vector<RenderingInfo>> RenderObjects;

	// レンダリング
public:
	static Material::RenderingTiming GetCurrentRenderingTiming() { return CurrentRenderingTiming; }
private:
	void DepthNormalRendering();
	void DefferedRendering();
	void DefferedLighting();
	void ForwardRendering();
	void ViewDepthNormal();
	void ViewGBuffers();
private:
	static Material::RenderingTiming CurrentRenderingTiming;

};

#endif