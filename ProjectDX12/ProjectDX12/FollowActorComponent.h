#ifndef ___FOLLOW_ACTOR_COMPONENT_H___
#define ___FOLLOW_ACTOR_COMPONENT_H___

#include <memory>

// Component
#include "Component.h"

class FollowActorComponent : public Component
{
public:
	using Component::Component;

	void Init() override;
	void Update() override;
	
public:
	void SetFollowActor(std::shared_ptr<class GameObject> actor) { FollowActor = actor; }

private:
	// í«è]ëŒè€
	std::weak_ptr<class GameObject> FollowActor;

};

#endif