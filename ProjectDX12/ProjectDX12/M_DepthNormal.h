#ifndef ___M_DEPTH_NORMAL_H___
#define ___M_DEPTH_NORMAL_H___

#include "Material.h"

class M_DepthNormal : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap) override;
	virtual void Bind() override;

};

#endif