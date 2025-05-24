
#include "PostProcess.h"

PostProcess::~PostProcess()
{
}

void PostProcess::Update()
{
	Volumes.remove_if(
		[](Volume* object) {return object->Destroy(); });
}

void PostProcess::Draw()
{
	for (std::list<Volume*>::iterator it = Volumes.begin();
		it != Volumes.end(); ++it)
	{
		(*it)->Draw();
	}
}
