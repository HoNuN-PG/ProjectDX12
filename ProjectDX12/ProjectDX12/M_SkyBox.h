#ifndef ___M_SKY_BOX_H___
#define ___M_SKY_BOX_H___

#include "Material.h"

class M_SkyBox : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap) override;
	virtual void Bind() override;

};

#endif