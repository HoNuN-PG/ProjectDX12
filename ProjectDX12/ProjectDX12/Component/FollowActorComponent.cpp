

#include "Component/FollowActorComponent.h"
#include "System/GameObject/GameObject.h"

void FollowActorComponent::Init()
{
}

void FollowActorComponent::Update()
{
	// コンポーネントは所有元のGameObjectが保持しているため、Ownerは常に有効
	std::shared_ptr<GameObject> owner = Owner.lock();
	assert(owner);
	// 追従対象は別オブジェクトなので先に破棄されうる。消えていたら何もしない
	std::shared_ptr<GameObject> actor = FollowActor.lock();
	if (!actor)
	{
		return;
	}

	owner->SetPosition(actor->GetPosition());
}
