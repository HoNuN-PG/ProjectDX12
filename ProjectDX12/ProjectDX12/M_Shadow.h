#ifndef ___M_SHADOW_MAPS_H___
#define ___M_SHADOW_MAPS_H___

// Material
#include "Material.h"

namespace ShadowParam
{
	struct ShadowMapsParam
	{
		DirectX::XMFLOAT4X4 LVP;
	};
	struct ShadowReceieverParam
	{
		DirectX::XMFLOAT4X4 LVP[3];
		DirectX::XMFLOAT4 CascadeAreas;
	};
}

class M_ShadowMapsBase : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap, Description desc) override {};
	virtual void Bind() override {};

public:
	// 現在設定されているシャドウマップの番号
	static UINT CurrentShadowMapsNo;

};

class M_SimpleShadowMaps : public M_ShadowMapsBase
{
public:
	virtual void Initialize(DescriptorHeap* heap, Description desc) override;
	virtual void Bind() override;

};

class M_OpaqueSimpleShadowMaps : public M_ShadowMapsBase
{
public:
	virtual void Initialize(DescriptorHeap* heap, Description desc) override;
	virtual void Bind() override;

private:
	CommonParam common;

};

class M_ShadowRecieverBase : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap, Description desc) override;
	virtual void Bind() override;

protected:
	std::vector<std::shared_ptr<RenderTarget>> ShadowMaps;

};

class M_ShadowVSMRecieverBase : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap, Description desc) override;
	virtual void Bind() override;

protected:
	std::vector<std::shared_ptr<RenderTarget>> ShadowMaps;

};

#endif