#ifndef ___RENDERING_PASS_H___
#define ___RENDERING_PASS_H___

#include <memory>
#include <stdio.h>
#include <Windows.h>

#include "DescriptorHeap.h"

class GameObject;
class RenderTarget;

class RenderingPass
{
public:
	enum RenderingPassType
	{
		None = 0,
		OpaqueDepthNormal,
		CustomDepth,

		MAX_RENDERING_PASS_TYPE
	};
public:
	RenderingPassType GetType() { return PassType; }
protected:
	RenderingPassType PassType;
public:
	RenderingPass();
	virtual ~RenderingPass() {};
	virtual void Execute() {};
public:
	/**
	* @fn パスにオブジェクトを追加
	*/
	virtual void AddObj(GameObject& obj) {};
	/**
	* @fn パスのテクスチャを取得
	*/
	virtual std::shared_ptr<RenderTarget> GetTexture(UINT idx) = 0;
	virtual DescriptorHeap::Handle GetTextureRTV(UINT idx) = 0;
	virtual DescriptorHeap::Handle GetTextureSRV(UINT idx) = 0;

protected:
	static std::shared_ptr<class RenderingEngine> Engine;

};

#endif
