#ifndef ___M_SIMPLE_LIT_H___
#define ___M_SIMPLE_LIT_H___

#include "Material/Material.h"

class M_SimpleLit : public Material
{
public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;

};

class M_OpaqueSimpleLit : public Material
{
public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;

private:
	CommonParam common;

};

#endif