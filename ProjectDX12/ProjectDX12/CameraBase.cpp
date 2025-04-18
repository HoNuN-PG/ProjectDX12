
#include "CameraBase.h"

DirectX::XMFLOAT3	cCameraBase::m_MainPos;
DirectX::XMFLOAT3	cCameraBase::m_MainUp;
DirectX::XMFLOAT3	cCameraBase::m_MainTarget;

void cCameraBase::SetMainParams()
{
	m_MainPos		= m_Pos;
	m_MainUp		= m_Up;
	m_MainTarget	= m_Target;
}
