#ifndef ___COMPONENT_H___
#define ___COMPONENT_H___

#include <memory>

class Component
{
protected:
	std::weak_ptr<class GameObject> Owner;

public:

	Component() = delete;
	Component(std::shared_ptr<class GameObject> Object) { Owner = Object; }
	virtual ~Component() {}

	virtual void Init() {};
	virtual void Uninit() {};
	virtual void Update() {};
	virtual void Draw() {};

};

#endif