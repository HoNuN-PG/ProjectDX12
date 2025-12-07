#ifndef ___M_DEPTH_NORMAL_H___
#define ___M_DEPTH_NORMAL_H___

// Material
#include "Material.h"

class M_DepthNormal : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap, Description desc) override;
	virtual void Bind() override;

};

#endif