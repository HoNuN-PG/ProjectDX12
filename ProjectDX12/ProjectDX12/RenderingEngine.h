#ifndef ___RENDERING_ENGINE_H___
#define ___RENDERING_ENGINE_H___

#include <vector>
#include <memory>

#include "DescriptorHeap.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "ConstantBuffer.h"

#include <unordered_map>

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
	{
		return GlobalHeap.get();
	}
private:
	static std::unique_ptr<DescriptorHeap> GlobalHeap;

	// テクスチャヒープ
private:
	std::shared_ptr<DescriptorHeap>	RTVHeap;
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

	// リソース
private:
	void WriteGlobalConstantBufferResource();
private:
	CameraDebug* Camera;
	LightBase* Light;
private:
	std::unique_ptr<DepthStencil> DSV;

	// レンダリングオブジェクトの追加
public:
	void AddRenderObject(GameObject& obj, int timing);
private:
	std::vector<std::vector<RenderingInfo>> m_RenderObjects;

	// レンダリング
private:
	void DefferedRendering();
	void DefferedLighting();
	void ForwardRendering();
	void ViewGBuffers();

};

#endif