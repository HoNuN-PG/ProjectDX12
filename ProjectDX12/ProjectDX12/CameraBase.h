#ifndef ___CAMERA_BASE_H___
#define ___CAMERA_BASE_H___

#include <DirectXMath.h>
#include <Windows.h>

#define CAM_NEAR (0.01)
#define CAM_FAR (1000)

class cCameraBase
{
public:

	cCameraBase() {}
	virtual ~cCameraBase() {}
	virtual void Update() {}
	virtual void Draw(){}

	// 座標
	DirectX::XMFLOAT3 GetPos() { return m_Pos; }

protected:

	bool m_IsMain;

	DirectX::XMFLOAT3 m_Pos;			// カメラ座標
	DirectX::XMFLOAT3 m_Up;				// アップベクトル
	DirectX::XMFLOAT3 m_Target;			// 注視点

public:

	float m_MoveSpeed;
	float m_MouseSpeed;

protected:

	void SetMainParams();

public:

	static DirectX::XMFLOAT3 m_MainPos;
	static DirectX::XMFLOAT3 m_MainUp;
	static DirectX::XMFLOAT3 m_MainTarget;

};

class cCameraDebug : public cCameraBase
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
	cCameraDebug();
	~cCameraDebug() {}
	void Update() override;
	void Draw() override;
private:
	void ProcDCC(Argument& arg);
private:
	POINT m_oldPos;
};

#endif
