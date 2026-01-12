
// Game/Camera
#include "CameraBase.h"

// Lib
#include "MyMath.h"

DirectX::XMFLOAT3	CameraBase::m_MainPosition;
DirectX::XMFLOAT3	CameraBase::m_MainUp;
DirectX::XMFLOAT3	CameraBase::m_MainTarget;
DirectX::XMFLOAT4X4	CameraBase::m_MainViewMatrix;
DirectX::XMFLOAT4X4	CameraBase::m_MainProjMatrix;

void CameraBase::SetMainParams()
{
	if (!m_IsMain) return;

	m_MainPosition = m_Position;
	m_MainUp = m_Up;
	m_MainTarget = m_Target;
	m_MainViewMatrix = m_ViewMatrix;
	m_MainProjMatrix = m_ProjMatrix;
}

DirectX::XMFLOAT4X4 CameraBase::GetMainViewProjectionInvMatrix()
{
	DirectX::XMMATRIX vpInv;
	DirectX::XMMATRIX vM = DirectX::XMLoadFloat4x4(&m_MainViewMatrix);
	DirectX::XMMATRIX pM = DirectX::XMLoadFloat4x4(&m_MainProjMatrix);
	vpInv = DirectX::XMMatrixMultiply(vM, pM);
	vpInv = DirectX::XMMatrixInverse(nullptr, vpInv);

	DirectX::XMFLOAT4X4 vpInv4x4;
	DirectX::XMStoreFloat4x4(&vpInv4x4, vpInv);

	return vpInv4x4;
}

std::vector<DirectX::XMFLOAT4> CameraBase::GetMainFrustumPlanes()
{
	// フラスタムの法線の計算
	DirectX::XMFLOAT4 frustum[6];
	float fov = GetViewAngle();
	frustum[0] = { 0,-cosf(fov / 2.0f),sinf(fov / 2.0f),0 };				// 上
	frustum[1] = { 0, cosf(fov / 2.0f),sinf(fov / 2.0f),0 };				// 下
	frustum[2] = { cosf(fov / 2.0f),0,GetAspect() * sinf(fov / 2.0f),0 };	// 左
	frustum[3] = { -cosf(fov / 2.0f),0,GetAspect() * sinf(fov / 2.0f),0 };	// 右
	frustum[4] = { 0,0, 1,-CAM_NEAR }; // 前
	frustum[5] = { 0,0,-1 ,CAM_FAR };  // 奥

	DirectX::XMVECTOR vFrustum[6];
	for (int i = 0; i < 6; ++i)
	{
		vFrustum[i] = DirectX::XMLoadFloat4(&frustum[i]);
	}

	// ワールド変換
	DirectX::XMFLOAT3 z = 
	{
		m_MainTarget.x - m_MainPosition.x,
		m_MainTarget.y - m_MainPosition.y,
		m_MainTarget.z - m_MainPosition.z
	};
	z = DXFL::Normalize(z);
	DirectX::XMFLOAT3 x = DXFL::Cross(m_MainUp,z);
	x = DXFL::Normalize(x);
	DirectX::XMFLOAT3 y = DXFL::Cross(z, x);
	y = DXFL::Normalize(y);

	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
	world._11 = x.x; world._12 = x.y; world._13 = x.z;
	world._21 = y.x; world._22 = y.y; world._23 = y.z;
	world._31 = z.x; world._32 = z.y; world._33 = z.z;
	world._41 = m_MainPosition.x; world._42 = m_MainPosition.y; world._43 = m_MainPosition.z;
	DirectX::XMMATRIX mW = DirectX::XMLoadFloat4x4(&world);
	mW = DirectX::XMMatrixInverse(nullptr, mW);
	mW = DirectX::XMMatrixTranspose(mW);
	for (int i = 0; i < 6; ++i)
	{
		vFrustum[i] = DirectX::XMPlaneTransform(vFrustum[i], mW);
	}

	std::vector<DirectX::XMFLOAT4> planes;
	for (int i = 0; i < 6; ++i)
	{
		DirectX::XMFLOAT4 plane;
		DirectX::XMStoreFloat4(&plane, vFrustum[i]);
		planes.push_back(plane);
	}

	return planes;
}
