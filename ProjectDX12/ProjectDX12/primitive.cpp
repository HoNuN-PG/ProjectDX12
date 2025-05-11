
#include "primitive.h"
#include "GameObject.h"
#include "ConstantWVP.h"

#include "RenderingEngine.h"

void Primitive::Draw()
{
	for (auto material : MaterialData)
	{
		Owner->BindRenderingEngine(material->GetRenderingTiming());
	}
}

void Primitive::Rendering()
{
	Material::RenderingTiming current = RenderingEngine::GetCurrentRenderingTiming();
	for (auto material : MaterialData)
	{
		if (material->GetRenderingTiming() == current)
		{
			material->WriteWVP(ConstantWVP::Calc3DMatrix(
				Owner->GetPosition(),
				Owner->GetRotation(),
				Owner->GetScale()));
			material->Draw();
			break;
		}
	}
	MeshData->Draw();
}

void Primitive::AddMaterial(Material* material)
{
	MaterialData.push_back(material);
}
