
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
	std::shared_ptr<RenderingEngine> engine = SceneManager::GetRenderingEngine();
	assert(engine);
	Material::RenderingTiming current = engine->GetCurrentRenderingTiming();
	std::vector<MaterialRegistry::MeshMaterialInfo> infos = pMaterialRegistry->GetRenderingMaterial(current);

	// コンポーネントは所有元のGameObjectが保持しているため、Ownerは常に有効
	std::shared_ptr<GameObject> owner = Owner.lock();
	assert(owner);

	for (auto&& info : infos)
	{
		// マテリアル設定
		info.material->WriteWVP(CalcConstantWVP::Calc3DMatrix(
			owner->GetPosition(),
			owner->GetRotation(),
			owner->GetScale()),
			info.materialInstanceIdx
		);
		info.material->Bind(info.materialInstanceIdx);
		// 描画
		pMesh[info.meshIdx]->Draw();
		if (info.meshIdx == 1)
		{
			int a = 0;
		}
	}
}
