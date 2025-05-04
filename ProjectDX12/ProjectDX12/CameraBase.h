#ifndef ___CAMERA_BASE_H___
#define ___CAMERA_BASE_H___

#include <DirectXMath.h>
#include <Windows.h>

#include "GameObject.h"

#define CAM_NEAR (0.01)
#define CAM_FAR (1000)

class cCameraBase : public GameObject
{
public:
	cCameraBase() {}
	virtual ~cCameraBase() {}
	virtual void Init() override {}
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override {}

	// 座標
	DirectX::XMFLOAT3 GetPos() 
	{ return m_Pos; }

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

class CameraDebug : public cCameraBase
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

private:
	POINT m_oldPos;

};

#endif
