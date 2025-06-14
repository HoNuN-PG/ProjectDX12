#ifndef ___RENDERING_PASS_H___
#define ___RENDERING_PASS_H___

#include <memory>
#include <stdio.h>

class GameObject;

class RenderingPass
{
public:
	enum RenderingPassType
	{
		None = 0,
		OpaqueDepthNormal,

		MAX_RENDERING_PASS_TYPE
	};
public:
	RenderingPassType GetType() { return PassType; }
protected:
	RenderingPassType PassType;
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
