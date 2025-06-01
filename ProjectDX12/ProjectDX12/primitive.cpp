
#include "primitive.h"
#include "GameObject.h"
#include "ConstantWVP.h"

#include "RenderingEngine.h"

void Primitive::Draw()
{
	for (auto material : MaterialData)
	{
		Owner->BindRenderingEngine(material->GetRenderingPassType(), material->GetRenderingTiming());
	}
}

void Primitive::Rendering()
{
	Material::RenderingPassType current = RenderingEngine::GetCurrentRenderingPass();
	for (auto material : MaterialData)
	{
		if (material->GetRenderingPassType() == current)
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
