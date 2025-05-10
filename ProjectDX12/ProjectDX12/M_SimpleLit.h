#ifndef ___M_SIMPLE_LIT_H___
#define ___M_SIMPLE_LIT_H___

#include "Material.h"

class M_SimpleLit : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap, RenderingTiming timing = RenderingTiming::FORWARD) override;
	virtual void Draw() override;

};

#endif