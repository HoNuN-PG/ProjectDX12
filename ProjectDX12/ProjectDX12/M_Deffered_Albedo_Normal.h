#ifndef ___M_DEFFERED_ALBEDO_NORMAL_H___
#define ___M_DEFFERED_ALBEDO_NORMAL_H___

#include "Material.h"

class M_Deffered_Albedo_Normal : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap, RenderingTiming timing = RenderingTiming::FORWARD) override;
	virtual void Draw() override;

};

#endif