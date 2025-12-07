#ifndef __SCENE_SAND_BOX_DX12_H__
#define __SCENE_SAND_BOX_DX12_H__

#include <memory>
#include <vector>

// Material
#include "Material.h"

// Scene
#include "SceneBase.h"

class SceneSandBoxDX12 : public SceneBase
{
public:
	HRESULT Init();
	void Uninit();
	void Update();
	void Draw();

};

#endif