#ifndef ___M_MS_H___
#define ___M_MS_H___

// Material
#include "Material.h"

class M_MS : public Material
{
public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;
	virtual UINT GetMeshShaderSRVStartSlot() const override { return MeshShaderSRVStartSlot; }

private:
	void CalcMeshShaderSRVStartSlot(RootSignature::Parameter* parameters, UINT num);

private:
	UINT MeshShaderSRVStartSlot;

};

#endif