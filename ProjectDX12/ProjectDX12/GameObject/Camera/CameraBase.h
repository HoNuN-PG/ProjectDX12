#ifndef ___CAMERA_BASE_H___
#define ___CAMERA_BASE_H___

#include <DirectXMath.h>
#include <Windows.h>

#include "System/GameObject/GameObject.h"
#include "System/DirectX.h"
#include "System/MyMath.h"

#define CAM_NEAR (0.01)
#define CAM_FAR (1000)
#define VIEW_ANGLE (60)

class CameraBase : public GameObject
{
public:

	CameraBase();
	virtual ~CameraBase() {}
	virtual void Init() override {}
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override {}

	// ===========================================
	// メインカメラのパラメータ
public:
	void SetIsMain(bool main) { m_IsMain = main; }
protected:
	void SetMainParam();
protected:
	bool m_IsMain = false;

public:
	static DirectX::XMFLOAT4X4 GetMainViewProjectionInvMatrix();
	static std::vector<DirectX::XMFLOAT4> GetMainFrustumPlanes();
	static float GetViewAngle() { return DirectX::XMConvertToRadians(VIEW_ANGLE); }
	static float GetAspect() { return (float)WINDOW_WIDTH / WINDOW_HEIGHT; }

public:
	static DirectX::XMFLOAT3 m_MainPosition;
	static DirectX::XMFLOAT3 m_MainTarget;
	static DirectX::XMFLOAT3 m_MainUp;
	static DirectX::XMFLOAT4X4 m_MainViewMatrix;
	static DirectX::XMFLOAT4X4 m_MainProjMatrix;

public:
	DirectX::XMFLOAT3 GetPosition() 
	{ 
		return m_Position; 
	}
	DirectX::XMFLOAT3 GetTarget()
	{
		return m_Target;
	}
	DirectX::XMFLOAT3 GetUp()
	{
		return m_Up;
	}
	DirectX::XMFLOAT3 GetForward()
	{
		return DXFL::Normalize(DXFL::Subtraction(m_Target,m_Position));
	}
	DirectX::XMFLOAT3 GetRight()
	{
		return DXFL::Cross(DXFL::Normalize(m_Up), GetForward());
	}

protected:
	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_Target;
	DirectX::XMFLOAT3 m_Up;
	DirectX::XMFLOAT4X4 m_ViewMatrix;
	DirectX::XMFLOAT4X4 m_ProjMatrix;

	// 移動速度
public:
	void DrawImGUI();
public:
	static float m_MoveSpeed;
	static float m_MouseSpeed;

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
