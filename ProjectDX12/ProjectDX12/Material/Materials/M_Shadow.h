#ifndef ___M_SHADOW_MAPS_H___
#define ___M_SHADOW_MAPS_H___

#include "Material/Material.h"

namespace ShadowParam
{
	// シャドウマップのカスケード数（シェーダー側の配列数と合わせる）
	constexpr UINT CASCADE_COUNT = 3;

	struct ShadowMapsParam
	{
		DirectX::XMFLOAT4X4 LVP;
	};
	struct ShadowReceieverParam
	{
		DirectX::XMFLOAT4X4 LVP[CASCADE_COUNT];
		DirectX::XMFLOAT4 CascadeAreas;
	};
}

class M_ShadowMapsBase : public Material
{
public:
	virtual void Initialize(Description desc) override {};
	virtual void Bind(UINT materialinstance) override {};

};

class M_SimpleShadowMaps : public M_ShadowMapsBase
{
public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;

};

class M_OpaqueSimpleShadowMaps : public M_ShadowMapsBase
{
public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;

private:
	CommonParam common;

};

class M_ShadowRecieverBase : public Material
{
public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;

protected:
	std::vector<DescriptorHeap::Handle> ShadowMapSRVs;	// パスのシャドウマップのSRVのコピー先

};

class M_ShadowVSMRecieverBase : public Material
{
public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;

protected:
	std::vector<DescriptorHeap::Handle> ShadowMapSRVs;	// パスのシャドウマップのSRVのコピー先

};

#endif