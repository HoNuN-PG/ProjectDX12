
#include "primitive.h"
#include "GameObject.h"
#include "ConstantWVP.h"

#include "RenderingEngine.h"

void Primitive::Draw()
{
	for (auto material : MaterialData)
	{
		Owner->BindRenderingEngine(material->GetRenderTiming());
	}
}

void Primitive::Rendering()
{
	Material::RenderingTiming current = RenderingEngine::GetCurrentRenderingPass();
	for (auto material : MaterialData)
	{
		if (material->GetRenderTiming() == current)
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

void Primitive::AddMaterial(std::shared_ptr<Material> material)
{
	MaterialData.push_back(material);
}
