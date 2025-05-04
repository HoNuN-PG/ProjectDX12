#ifndef ___COMPONENT_H___
#define ___COMPONENT_H___

class Component
{
protected:
	class  GameObject* Owner = nullptr;

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