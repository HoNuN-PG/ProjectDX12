#ifndef ___M_SHADOW_MAPS_H___
#define ___M_SHADOW_MAPS_H___

#include "Material.h"

namespace ShadowParam
{
	struct ShadowMapsParam
	{
		DirectX::XMFLOAT4X4 LVP;
	};
	struct ShadowReceieveParam
	{
		DirectX::XMFLOAT4X4 LVP[3];
	};
}

class M_SimpleShadowMaps : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap) override;
	virtual void Bind() override;

};

#endif