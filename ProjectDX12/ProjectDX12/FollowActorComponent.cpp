
#include "FollowActorComponent.h"

#include "GameObject.h"

void FollowActorComponent::Init()
{
}

void FollowActorComponent::Update()
{
	Owner.lock()->SetPosition(FollowActor.lock()->GetPosition());
}
