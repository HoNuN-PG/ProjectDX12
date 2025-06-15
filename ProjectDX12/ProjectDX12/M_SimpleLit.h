#ifndef ___M_SIMPLE_LIT_H___
#define ___M_SIMPLE_LIT_H___

#include "Material.h"

class M_SimpleLit : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap,
		RenderingTiming timing = RenderingTiming::Forward) override;
	virtual void Bind() override;

};

#endif