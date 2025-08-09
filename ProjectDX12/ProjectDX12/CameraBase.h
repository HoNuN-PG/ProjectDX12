#ifndef ___CAMERA_BASE_H___
#define ___CAMERA_BASE_H___

#include <DirectXMath.h>
#include <Windows.h>

#include "DirectX.h"
#include "MyMath.h"

#include "GameObject.h"

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

	// 座標
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

public:
	static DirectX::XMFLOAT4X4 m_ViewMatrix;
	static DirectX::XMFLOAT4X4 m_ProjMatrix;
	static float GetViewAngle() { return DirectX::XMConvertToRadians(VIEW_ANGLE); }
	static float GetAspect() { return (float)WINDOW_WIDTH / WINDOW_HEIGHT; }

public:
	float m_MoveSpeed;
	float m_MouseSpeed;

protected:
	void SetMainParams();
protected:
	bool m_IsMain;

	DirectX::XMFLOAT3 m_Pos;			// カメラ座標
	DirectX::XMFLOAT3 m_Up;				// アップベクトル
	DirectX::XMFLOAT3 m_Target;			// 注視点

public:
	static DirectX::XMFLOAT3 m_MainPos;
	static DirectX::XMFLOAT3 m_MainUp;
	static DirectX::XMFLOAT3 m_MainTarget;

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
	virtual void Update() override;
	virtual void Draw() override;

private:
	void ProcDCC(Argument& arg);

public:
	static DirectX::XMFLOAT4X4 GetCustomProjMatrix(UINT32 with, UINT32 height);
	static DirectX::XMFLOAT4X4 GetCustomProjMatrix_Perspective(UINT32 with, UINT32 height);

private:
	POINT m_oldPos;

};

#endif
