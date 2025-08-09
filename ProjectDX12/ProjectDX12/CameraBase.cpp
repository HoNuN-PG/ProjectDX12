
#include "CameraBase.h"

DirectX::XMFLOAT3	CameraBase::m_MainPos;
DirectX::XMFLOAT3	CameraBase::m_MainUp;
DirectX::XMFLOAT3	CameraBase::m_MainTarget;
DirectX::XMFLOAT4X4	CameraBase::m_ViewMatrix;
DirectX::XMFLOAT4X4	CameraBase::m_ProjMatrix;

void CameraBase::SetMainParams()
{
	m_MainPos		= m_Pos;
	m_MainUp		= m_Up;
	m_MainTarget	= m_Target;
}
