#ifndef ___M_GRID_H___
#define ___M_GRID_H___

#include "Material.h"

struct GridParam
{
	float GridSize;
	float SubGridNum;
	DirectX::XMFLOAT2 pad1;
	DirectX::XMFLOAT4 ObjectPositionWS;
};

class M_Grid : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap,
		RenderingTiming timing = RenderingTiming::Forward) override;
	virtual void Bind() override;

public:
	void SetGridSize(float size) { Grid.GridSize = size; }
	void SetSubGridNum(float num) { Grid.SubGridNum = num; }
private:
	GridParam Grid;

};

#endif