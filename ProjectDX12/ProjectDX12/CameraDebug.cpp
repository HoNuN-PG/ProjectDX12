
#include "CameraBase.h"

#include "imgui/imgui.h"

#include "DirectX.h"
#include "Input.h"

CameraDebug::CameraDebug()
{
	m_IsMain = true;

	m_Pos		= { 0,5,-10 };
	m_Up		= { 0,1,0 };
	m_Target	= { 0,0,0 };

	m_MoveSpeed		= 0.5;
	m_MouseSpeed	= 1.0f;

	SetMainParams();
}

void CameraDebug::Update()
{
	// マウス入力
	if (!Input::GetKeyPress(VK_RBUTTON))
	{
		GetCursorPos(&m_oldPos);
		return;
	}

	Argument arg;
	// マウス移動量
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	arg.mouseMove = DirectX::XMFLOAT2((float)cursorPos.x - m_oldPos.x, (float)cursorPos.y - m_oldPos.y);
	m_oldPos = cursorPos;

	// カメラ情報
	arg.vCamPos = DirectX::XMLoadFloat3(&m_Pos);
	arg.vCamLook = DirectX::XMLoadFloat3(&m_Target);
	DirectX::XMVECTOR vCamUp = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&m_Up));
	DirectX::XMVECTOR vFront = DirectX::XMVectorSubtract(arg.vCamLook, arg.vCamPos);
	// カメラ姿勢
	arg.vCamFront = DirectX::XMVector3Normalize(vFront);
	arg.vCamSide = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vCamUp, arg.vCamFront));
	arg.vCamUp = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(arg.vCamFront, arg.vCamSide));
	// フォーカス距離
	DirectX::XMStoreFloat(&arg.focus, DirectX::XMVector3Length(vFront));

	ProcDCC(arg);

	// メインカメラパラメータの設定
	if (m_IsMain) 
	{
		SetMainParams();
	}
	
	// マトリクス計算
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat3(&m_MainPos),
		DirectX::XMLoadFloat3(&m_MainTarget),
		XMLoadFloat3(&m_MainUp));
	view = DirectX::XMMatrixTranspose(view);
	DirectX::XMStoreFloat4x4(&m_MainViewMatrix, view);

	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(
		GetViewAngle(),
		(float)WINDOW_WIDTH / WINDOW_HEIGHT, 
		(float)CAM_NEAR, 
		(float)CAM_FAR);
	proj = DirectX::XMMatrixTranspose(proj);
	DirectX::XMStoreFloat4x4(&m_MainProjMatrix, proj);
}

void CameraDebug::Draw()
{
	ImGui::Begin("Camera");
	{
		ImGui::SliderFloat("Camera_MoveSpeed", &m_MoveSpeed, 0.0f, 5.0f);
		ImGui::SliderFloat("Camera_MouseSpeed", &m_MouseSpeed, 0.0f, 5.0f);
	}
	ImGui::End();
}

void CameraDebug::ProcDCC(Argument& arg)
{
	// マウスの移動量 / 画面サイズ の比率から、画面全体でどれだけ回転するか指定する。
	float angleX = 360.0f * arg.mouseMove.x / WINDOW_WIDTH * m_MouseSpeed;
	float angleY = 180.0f * arg.mouseMove.y / WINDOW_HEIGHT * m_MouseSpeed;

	// 横回転
	DirectX::XMMATRIX matUpRot = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(angleX));
	DirectX::XMVECTOR vSideAxis = DirectX::XMVector3Normalize(DirectX::XMVector3TransformCoord(arg.vCamSide, matUpRot));

	// 縦回転
	DirectX::XMMATRIX matSideRot = DirectX::XMMatrixRotationAxis(vSideAxis, DirectX::XMConvertToRadians(angleY));
	DirectX::XMVECTOR vFrontAxis = DirectX::XMVector3Normalize(DirectX::XMVector3TransformCoord(arg.vCamFront, matUpRot * matSideRot));

	// キー入力で移動
	DirectX::XMVECTOR vCamMove = DirectX::XMVectorZero();
	if (Input::GetKeyPress('W')) vCamMove = DirectX::XMVectorAdd(vCamMove, vFrontAxis);
	if (Input::GetKeyPress('S')) vCamMove = DirectX::XMVectorSubtract(vCamMove, vFrontAxis);
	if (Input::GetKeyPress('A')) vCamMove = DirectX::XMVectorSubtract(vCamMove, vSideAxis);
	if (Input::GetKeyPress('D')) vCamMove = DirectX::XMVectorAdd(vCamMove, vSideAxis);
	if (Input::GetKeyPress('Q')) vCamMove = DirectX::XMVectorAdd(vCamMove, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	if (Input::GetKeyPress('E')) vCamMove = DirectX::XMVectorAdd(vCamMove, DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));
	vCamMove = DirectX::XMVectorScale(vCamMove, m_MoveSpeed);

	// 更新
	DirectX::XMVECTOR vCamPos = DirectX::XMVectorAdd(arg.vCamPos, vCamMove);
	DirectX::XMStoreFloat3(&m_Pos, vCamPos);
	DirectX::XMStoreFloat3(&m_Target, DirectX::XMVectorAdd(vCamPos, DirectX::XMVectorScale(vFrontAxis, arg.focus)));
	DirectX::XMStoreFloat3(&m_Up, DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vFrontAxis, vSideAxis)));

	// Transform更新
	SetPosition(m_Pos);
}

DirectX::XMFLOAT4X4 CameraDebug::GetCustomProjMatrix(UINT32 with, UINT32 height)
{
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(
		GetViewAngle(),
		(float)with / height,
		(float)CAM_NEAR,
		(float)CAM_FAR);
	proj = DirectX::XMMatrixTranspose(proj);
	DirectX::XMFLOAT4X4 mat;
	DirectX::XMStoreFloat4x4(&mat, proj);
	return mat;
}

DirectX::XMFLOAT4X4 CameraDebug::GetCustomProjMatrix_Perspective(UINT32 with, UINT32 height)
{
	DirectX::XMFLOAT4X4 mat;
	DirectX::XMStoreFloat4x4(&mat,
		DirectX::XMMatrixOrthographicLH(
			with,
			height,
			(float)CAM_NEAR,
			(float)CAM_FAR));
	return mat;
}
