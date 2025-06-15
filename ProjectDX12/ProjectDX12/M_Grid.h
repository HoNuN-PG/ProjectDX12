#ifndef ___M_GRID_H___
#define ___M_GRID_H___

#include "Material.h"

struct GridParam
{
	float dist;
	DirectX::XMFLOAT3 pad1;
};

class M_Grid : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap,
		RenderingTiming timing = RenderingTiming::Forward) override;
	virtual void Bind() override;

public:
	void SetDistance(float dist) { Grid.dist = dist; };
private:
	GridParam Grid;

};

#endif