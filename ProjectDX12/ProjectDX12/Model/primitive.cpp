
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
	RenderingEngine* engine = SceneManager::GetRenderingEngine();
	assert(engine);
	Material::RenderingTiming current = engine->GetCurrentRenderingTiming();
	std::vector<MaterialRegistry::MeshMaterialInfo> infos = pMaterialRegistry->GetRenderingMaterial(current);
	
	for (auto&& info : infos)
	{
		// ƒ}ƒeƒŠƒAƒ‹Ý’è
		info.material->WriteWVP(CalcConstantWVP::Calc3DMatrix(Owner->GetWorldMatrix()), info.materialInstanceIdx);
		info.material->Bind(info.materialInstanceIdx);
		// •`‰æ
		pMesh[info.meshIdx]->Draw();
	}
}
