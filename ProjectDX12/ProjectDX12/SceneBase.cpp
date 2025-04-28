
#include "SceneBase.h"

void SceneBase::SetUp()
{
	Camera = std::make_unique<CameraDebug>();
	Light = std::make_unique<LightBase>();
}
