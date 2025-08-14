
#include "primitive.h"
#include "GameObject.h"
#include "ConstantWVP.h"

#include "RenderingEngine.h"

void Primitive::Draw()
{
	for (int i = 0; i < MeshMaterialsData.size(); ++i)
	{
		for (auto material : MeshMaterialsData[i])
		{
			Owner.lock()->BindRenderingEngine(material->GetRenderTiming(), material->GetPassType());
		}
	}
}

void Primitive::Rendering()
{
	Material::RenderingTiming current = RenderingEngine::GetCurrentRenderingPass();
	for (int i = 0; i < MeshData.size(); ++i)
	{
		for (auto material : MeshMaterialsData[i])
		{
			if (material->GetRenderTiming() == current)
			{
				material->WriteWVP(ConstantWVP::Calc3DMatrix(
					Owner.lock()->GetPosition(),
					Owner.lock()->GetRotation(),
					Owner.lock()->GetScale()));
				material->Bind();
				MeshData[i]->Draw();
				break;
			}
		}
	}
}
