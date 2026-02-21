#ifndef ___M_MS_H___
#define ___M_MS_H___

#include "Material/Material.h"

#define LANE_COUNT (32)

class M_MS : public Material
{
public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;
	virtual int GetMeshShaderSRVStartSlot() const override { return MeshShaderSRVStartSlot; }

protected:
	void CalcMeshShaderStartSlot_SRV(RootSignature::Parameter* parameters, UINT num);

private:
	int MeshShaderSRVStartSlot;

};

class M_MSCulling : public Material
{
public:
	struct MESHLET_INFO
	{
		UINT meshletCount;
		DirectX::XMFLOAT3 pad1;
	};

public:
	virtual void Initialize(Description desc) override;
	virtual void Bind(UINT materialinstance) override;
	virtual int GetAmpShaderSRVStartSlot() const override { return AmpShaderSRVStartSlot; }
	virtual int GetMeshShaderSRVStartSlot() const override { return MeshShaderSRVStartSlot; }
	virtual void WriteMeshletCount(int count) override;

protected:
	void CalcAmpShaderStartSlot_SRV(RootSignature::Parameter* parameters, UINT num);
	void CalcMeshShaderStartSlot_SRV(RootSignature::Parameter* parameters, UINT num);

private:
	int AmpShaderSRVStartSlot;
	int MeshShaderSRVStartSlot;

};

#endif