#ifndef __SCENE_SAND_BOX_DX12_H__
#define __SCENE_SAND_BOX_DX12_H__

#include "SceneBase.h"

#include <vector>
#include <memory>

#include "Material.h"

class SceneSandBoxDX12 : public SceneBase
{
public:
	HRESULT Init();
	void Uninit();
	void Update();
	void Draw();

private:
	std::vector<std::unique_ptr<Material>> Materials;

};

#endif