#ifndef ___M_GRID_H___
#define ___M_GRID_H___

#include "M_Shadow.h"

namespace Grid 
{
	struct GridParam
	{
		float GridSize;
		float SubGridSize;
		float GridWidth;
		float pad1;
	};
}

class M_Grid : public Material
{
public:
	virtual void Initialize(DescriptorHeap* heap) override;
	virtual void Bind() override;

public:
	void SetGridSize(float size) { GridParam.GridSize = size; }
	void SetSubGridSize(float size) { GridParam.SubGridSize = size; }
	void SetGridWidth(float width) { GridParam.GridWidth = width; }
private:
	Grid::GridParam GridParam;

};

class M_GridShadow : public M_ShadowRecieverBase
{
public:
	virtual void Initialize(DescriptorHeap* heap) override;
	virtual void Bind() override;

public:
	void SetGridSize(float size) { GridParam.GridSize = size; }
	void SetSubGridSize(float size) { GridParam.SubGridSize = size; }
	void SetGridWidth(float width) { GridParam.GridWidth = width; }
private:
	Grid::GridParam GridParam;
};

#endif