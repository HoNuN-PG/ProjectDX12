
#include "primitive.h"
#include "GameObject.h"
#include "ConstantWVP.h"

void Primitive::Draw()
{
	// WVP‚ÌÝ’è
	MaterialData->WriteWVP(ConstantWVP::Calc3DMatrix(
		Owner->GetPosition(),
		Owner->GetRotation(),
		Owner->GetScale()));
	MaterialData->Draw();
	MeshData->Draw();
}

void Primitive::SetMaterial(Material* material)
{
	MaterialData = material;
}
