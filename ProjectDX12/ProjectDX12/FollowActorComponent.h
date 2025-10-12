#ifndef ___FOLLOW_ACTOR_COMPONENT_H___
#define ___FOLLOW_ACTOR_COMPONENT_H___

#include <memory>

#include "Component.h"

class FollowActorComponent : public Component
{
public:
	using Component::Component;

	void Init() override;
	void Update() override;

	void SetFollowActor(std::shared_ptr<class GameObject> actor) { FollowActor = actor; }

private:
	std::weak_ptr<class GameObject> FollowActor;
};

#endif