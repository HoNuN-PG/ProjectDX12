
#include "System/Rendering/Volume/PostProcess.h"

PostProcess::~PostProcess()
{
}

void PostProcess::Update()
{
	Volumes.remove_if(
		[](const std::shared_ptr<Volume>& object) 
		{
			if (object->IsDestroy())
			{
				object->Destroy();
				return true;
			}
			return false; 
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
