
#include "Model/primitive.h"
#include "Scene/SceneManager.h"
#include "System/GameObject/GameObject.h"
#include "System/Rendering/ConstantBuffer/ConstantWVP.h"
#include "System/Rendering/RenderingEngine.h"

void Primitive::Draw()
{
	pMaterialRegistry->Register2RenderingEngine(Owner);
}

void Primitive::Rendering()
{
	std::weak_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	Material::RenderingTiming current = engine.lock()->GetCurrentRenderingTiming();
	std::vector<MaterialRegistry::MeshMaterialInfo> infos = pMaterialRegistry->GetRenderingMaterial(current);

	for (auto&& info : infos)
	{
		// ƒ}ƒeƒŠƒAƒ‹Ý’è
		info.material->WriteWVP(CalcConstantWVP::Calc3DMatrix(
			Owner.lock()->GetPosition(),
			Owner.lock()->GetRotation(),
			Owner.lock()->GetScale()),
			info.materialInstanceIdx
		);
		info.material->Bind(info.materialInstanceIdx);
		// •`‰æ
		pMesh[info.meshIdx]->Draw();
		if (info.meshIdx == 1)
		{
			int a = 0;
		}
	}
}
