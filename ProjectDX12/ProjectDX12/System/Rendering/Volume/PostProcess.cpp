
#include "System/Rendering/Volume/PostProcess.h"

PostProcess::~PostProcess()
{
}

void PostProcess::Update()
{
	Volumes.remove_if(
		[](std::shared_ptr<Volume> object) 
		{
			if (object->IsDestroy())
			{
				object->Destroy();
				object = nullptr;
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
