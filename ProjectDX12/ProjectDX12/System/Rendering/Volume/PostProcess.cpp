
#include "System/Rendering/Volume/PostProcess.h"

PostProcess::~PostProcess()
{
}

void PostProcess::Update()
{
	Volumes.remove_if(
		[](const std::unique_ptr<Volume>& object) 
		{
			return object->IsDestroy();
		}
	);
}

void PostProcess::Draw()
{
	for (auto&& it = Volumes.begin(); it != Volumes.end(); ++it)
	{
		(*it)->Draw();
	}
}
