#ifndef ___RENDERING_ENGINE_H___
#define ___RENDERING_ENGINE_H___

#include <vector>
#include <memory>

#include "DescriptorHeap.h"
#include "DepthStencil.h"
#include "ConstantBuffer.h"

#include <unordered_map>

class CameraDebug;
class LightBase;

class GameObject;

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

	// デプスステンシルバッファ
private:
	std::unique_ptr<DescriptorHeap>					DSVHeap;
	std::unique_ptr<DepthStencil>					DSV;

	// グローバルリソース
public:
	static DescriptorHeap* GetGlobalHeap()
	{
		return GlobalHeap.get();
	}
	static ConstantBuffer* GetGlobalResource(UINT key);
private:
	void WriteGlobalResource();
private:
	static std::unique_ptr<DescriptorHeap>								GlobalHeap;
	static std::unordered_map<UINT, std::unique_ptr<ConstantBuffer>>	GlobalResource;
private:
	CameraDebug* Camera;
	LightBase* Light;

public:
	void AddRenderObject(GameObject& obj, int timing);

private:
	void ForwardRendering();

private:
	std::vector<std::vector<RenderingInfo>> m_RenderObjects;

};

#endif