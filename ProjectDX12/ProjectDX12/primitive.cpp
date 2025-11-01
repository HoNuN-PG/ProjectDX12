
#include "primitive.h"

#include "SceneManager.h"

#include "GameObject.h"

#include "ConstantWVP.h"

#include "RenderingEngine.h"

void Primitive::Draw()
{
	MeshMaterial->BindRenderingEngine(Owner);
}

void Primitive::Rendering()
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	Material::RenderingTiming current = engine.lock()->GetCurrentRenderingTiming();
	UINT idx;
	if (auto ret = MeshMaterial->GetMeshMaterial(current, idx))
	{
		ret->WriteWVP(ConstantWVP::Calc3DMatrix(
			Owner.lock()->GetPosition(),
			Owner.lock()->GetRotation(),
			Owner.lock()->GetScale()));
		ret->Bind();
		MeshData[idx]->Draw();
	}
}
