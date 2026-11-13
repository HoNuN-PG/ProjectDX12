#ifndef ___M_DEFERRED_ALBEDO_H___
#define ___M_DEFERRED_ALBEDO_H___

#include "Material/Material.h"

class M_Deferred_Albedo : public Material
{
public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;

};

#endif