#ifndef ___M_SKY_BOX_H___
#define ___M_SKY_BOX_H___

// Material
#include "Material.h"

class M_SkyBox : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap, Description desc) override;
	virtual void Bind() override;

};

#endif