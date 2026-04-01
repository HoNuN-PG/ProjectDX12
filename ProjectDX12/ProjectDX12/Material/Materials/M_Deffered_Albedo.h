#ifndef ___M_DEFFERED_ALBEDO_H___
#define ___M_DEFFERED_ALBEDO_H___

#include "Material/Material.h"

class M_Deffered_Albedo : public Material
{
public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;

};

#endif