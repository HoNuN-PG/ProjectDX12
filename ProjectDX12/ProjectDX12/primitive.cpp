
#include "primitive.h"

#include "SceneManager.h"
#include "GameObject.h"

#include "RenderingEngine.h"
#include "ConstantWVP.h"

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
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	Material::RenderingTiming current = engine.lock()->GetCurrentRenderingPass();
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
