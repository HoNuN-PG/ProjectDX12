
// Game/Camera
#include "CameraBase.h"

DirectX::XMFLOAT3	CameraBase::m_MainPos;
DirectX::XMFLOAT3	CameraBase::m_MainUp;
DirectX::XMFLOAT3	CameraBase::m_MainTarget;
DirectX::XMFLOAT4X4	CameraBase::m_MainViewMatrix;
DirectX::XMFLOAT4X4	CameraBase::m_MainProjMatrix;

DirectX::XMFLOAT4X4 CameraBase::GetViewProjectionInvMatrix()
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

void CameraBase::SetMainParams()
{
	m_MainPos		= m_Pos;
	m_MainUp		= m_Up;
	m_MainTarget	= m_Target;
}
