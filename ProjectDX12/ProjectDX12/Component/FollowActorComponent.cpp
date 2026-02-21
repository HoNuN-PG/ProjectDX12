

#include "Component/FollowActorComponent.h"
#include "System/GameObject/GameObject.h"

void FollowActorComponent::Init()
{
}

void FollowActorComponent::Update()
{
	Owner.lock()->SetPosition(FollowActor.lock()->GetPosition());
}
