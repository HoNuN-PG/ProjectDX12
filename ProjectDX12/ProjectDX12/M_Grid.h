#ifndef ___M_GRID_H___
#define ___M_GRID_H___

#include "Material.h"

namespace Grid 
{
	struct GridParam
	{
		float GridSize;
		float SubGridNum;
		float GridWidth;
		float pad1;
	};
}

class M_Grid : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap,
		RenderingTiming timing = RenderingTiming::Forward) override;
	virtual void Bind() override;

public:
	void SetGridSize(float size) { GridParam.GridSize = size; }
	void SetSubGridNum(float num) { GridParam.SubGridNum = num; }
	void SetGridWidth(float width) { GridParam.GridWidth = width; }
private:
	Grid::GridParam GridParam;

};

#endif