#ifndef ___CAMERA_BASE_H___
#define ___CAMERA_BASE_H___

#include <DirectXMath.h>
#include <Windows.h>

// Lib
#include "MyMath.h"

// System/GameObject
#include "GameObject.h"
// System
#include "DirectX.h"

#define CAM_NEAR (0.01)
#define CAM_FAR (1000)
#define VIEW_ANGLE (60)

class CameraBase : public GameObject
{
public:
	CameraBase() {}
	virtual ~CameraBase() {}
	virtual void Init() override {}
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override {}

public:
	DirectX::XMFLOAT3 GetPosition() 
	{ 
		return m_Pos; 
	}
	DirectX::XMFLOAT3 GetForward()
	{
		return DXFL::Normalize(DXFL::Subtraction(m_Target,m_Pos));
	}
	DirectX::XMFLOAT3 GetRight()
	{
		return DXFL::Cross(DXFL::Normalize(m_Up), GetForward());
	}

protected:
	DirectX::XMFLOAT3 m_Pos;
	DirectX::XMFLOAT3 m_Up;
	DirectX::XMFLOAT3 m_Target;	

	// 移動速度
public:
	float m_MoveSpeed;
	float m_MouseSpeed;

	// 各種カメラパラメータ
public:
	static float GetViewAngle() { return DirectX::XMConvertToRadians(VIEW_ANGLE); }
	static float GetAspect() { return (float)WINDOW_WIDTH / WINDOW_HEIGHT; }

	// マトリクス
public:
	static DirectX::XMFLOAT4X4 GetViewProjectionInvMatrix();

	// メインカメラのパラメータ
protected:
	void SetMainParams();
public:
	static DirectX::XMFLOAT3 m_MainPos;
	static DirectX::XMFLOAT3 m_MainUp;
	static DirectX::XMFLOAT3 m_MainTarget;
	static DirectX::XMFLOAT4X4 m_MainViewMatrix;
	static DirectX::XMFLOAT4X4 m_MainProjMatrix;
protected:
	bool m_IsMain;

};

class CameraDebug : public CameraBase
{
private:
	struct Argument
	{
		DirectX::XMFLOAT2 mouseMove;
		DirectX::XMVECTOR vCamFront;
		DirectX::XMVECTOR vCamSide;
		DirectX::XMVECTOR vCamUp;
		DirectX::XMVECTOR vCamPos;
		DirectX::XMVECTOR vCamLook;
		float focus;
	};

public:
	CameraDebug();
	~CameraDebug() {}
	virtual void Init() override;
	virtual void Update() override;
	virtual void Draw() override;

private:
	void ProcDCC(Argument& arg);

private:
	POINT m_oldPos;

};

#endif
