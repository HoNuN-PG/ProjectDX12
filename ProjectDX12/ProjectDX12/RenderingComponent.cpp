
// Model
#include "RenderingComponent.h"

void RenderingComponent::ReuseRendering()
{
	MeshMaterial->ReuseRendering();
}

void RenderingComponent::RefreshRendering()
{
	MeshMaterial->RefreshRendering();
}