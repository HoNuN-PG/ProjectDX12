#ifndef ___RENDERING_PASS_H___
#define ___RENDERING_PASS_H___

#include <memory>

class GameObject;

class RenderingPass
{
public:
	enum RenderingPassType
	{
		Shadow = 0,					// シャドウ
		OpaqueDepthNormal,			// 不透明深度
		Forward,					// フォワード
		Deffered,					// ディファード
		TranslucentDepthNormal,		// 透明深度
		Other,						// その他

		MAX_RENDERING_PASS_TYPE
	};
public:
	RenderingPassType GetID() { return PassID; }
protected:
	RenderingPassType PassID;
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
