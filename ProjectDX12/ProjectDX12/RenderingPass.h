#ifndef ___RENDERING_PASS_H___
#define ___RENDERING_PASS_H___

#include <memory>

class GameObject;

class RenderingPass
{
public:
	enum RenderingPassType
	{
		SHADOW = 0,					// シャドウ
		O_DEPTH_NORMAL_PASS,		// 不透明深度
		MAIN,						// メイン
		T_DEPTH_NORMAL_PASS,		// 透明深度
		OTHER,						// その他

		MAX_RENDERING_PASS
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
