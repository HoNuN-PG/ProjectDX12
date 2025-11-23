
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
	MeshMaterialManager::MeshMaterialInfo info = MeshMaterial->GetMeshMaterial(current);
	if (info.material)
	{
		info.material->WriteWVP(ConstantWVP::Calc3DMatrix(
			Owner.lock()->GetPosition(),
			Owner.lock()->GetRotation(),
			Owner.lock()->GetScale()));
		info.material->Bind();
		MeshData[info.meshIdx]->Draw();
	}
}
