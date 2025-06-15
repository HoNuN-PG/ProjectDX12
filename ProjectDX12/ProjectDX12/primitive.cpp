
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
			material->Bind();
			break;
		}
	}
	MeshData->Draw();
}
