
#include "primitive.h"
#include "GameObject.h"
#include "ConstantWVP.h"

#include "RenderingEngine.h"

void Primitive::Draw()
{
	for (auto material : MaterialData)
	{
		Owner.lock()->BindRenderingEngine(material->GetRenderTiming(), material->GetPassType());
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
				Owner.lock()->GetPosition(),
				Owner.lock()->GetRotation(),
				Owner.lock()->GetScale()));
			material->Bind();
			break;
		}
	}
	MeshData->Draw();
}
