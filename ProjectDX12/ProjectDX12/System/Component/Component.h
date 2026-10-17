#ifndef ___COMPONENT_H___
#define ___COMPONENT_H___

#include <memory>

class Component
{
protected:
	class GameObject* Owner; // コンポーネントはGameObjectが所有するため参照が保証されている

public:

	Component() = delete;
	Component(class GameObject* Object) { Owner = Object; }
	virtual ~Component() {}

	virtual void Init() {};
	virtual void Uninit() {};
	virtual void Update() {};
	virtual void Draw() {};

};

#endif