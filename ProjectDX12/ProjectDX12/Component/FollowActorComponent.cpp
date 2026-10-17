

#include "Component/FollowActorComponent.h"
#include "System/GameObject/GameObject.h"

void FollowActorComponent::Init()
{
}

void FollowActorComponent::Update()
{
	// 追従対象は別オブジェクトなので先に破棄されうる。消えていたら何もしない
	std::shared_ptr<GameObject> actor = FollowActor.lock();
	if (!actor)
	{
		return;
	}

	Owner->SetPosition(actor->GetPosition());
}
