#ifndef ___RENDERING_PASS_H___
#define ___RENDERING_PASS_H___

#include <memory>
#include "Material.h"

class GameObject;

class RenderingPass
{
public:
	Material::RenderingPassType GetID() { return PassID; }
protected:
	Material::RenderingPassType PassID;
public:
	RenderingPass();
	virtual ~RenderingPass() {};
	virtual void Execute() {};
public:
	/**
	 * @fn パスにオブジェクトを追加
	 */
	virtual void AddObj(GameObject& obj) {};

protected:
	static std::shared_ptr<class RenderingEngine> Engine;

};

#endif
