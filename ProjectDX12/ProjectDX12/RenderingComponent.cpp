
// Model
#include "RenderingComponent.h"

void RenderingComponent::ReuseRendering()
{
	MeshMaterialData->Reuse();
}

void RenderingComponent::RefreshRendering()
{
	MeshMaterialData->Refresh();
}