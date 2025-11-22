#ifndef ___M_SIMPLE_LIT_H___
#define ___M_SIMPLE_LIT_H___

#include "Material.h"

class M_SimpleLit : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap, Description desc) override;
	virtual void Bind() override;

};

class M_OpaqueSimpleLit : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap, Description desc) override;
	virtual void Bind() override;

private:
	CommonParam common;

};

#endif