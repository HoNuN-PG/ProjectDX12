
#include "LightBase.h"
#include "Input.h"

DirectX::XMFLOAT4X4 cLightBase::m_ViewMatrix;
DirectX::XMFLOAT4X4 cLightBase::m_ProjectionMatrix;


const float LIG_SPEED_X = 3.0f;			// 回転速度
const float LIG_SPEED_Y = 2.5f;
const float LIG_LATE_SPEED_X = 1.0f;	// 補間強度
const float LIG_LATE_SPEED_Y = 1.0f;
const float MAX_ANGLE_UP = 85.0f;		// 最大上下回り込み角度
const float MAX_ANGLE_DOWN = 5.0f;

const float SPEED = 1;

cLightBase::cLightBase()
{
	// パラメータ設定
	m_Up = { 0,1,0 };
	m_Position = DirectX::XMFLOAT3(0, 1, 0);
	m_Target = DirectX::XMFLOAT3();
	m_TargetOffset = DirectX::XMFLOAT3();
	m_Dist = 350;
	m_Rad.radXZ = m_Rad.lateXZ = DirectX::XMConvertToRadians(0.0f);
	m_Rad.radY = m_Rad.lateY = DirectX::XMConvertToRadians((MAX_ANGLE_UP + MAX_ANGLE_DOWN) / 2.0f);
	m_Color = { 1,1,1,1 };
	m_Power = 1.5f;
	m_Ambient = 0.2f;

	// 角度と距離・注視点からカメラ位置を計算
	m_Position.x = cosf(m_Rad.lateY) * sinf(m_Rad.lateXZ) * (m_Dist)+m_Target.x;
	m_Position.y = sinf(m_Rad.lateY) * (m_Dist)+m_Target.y;
	m_Position.z = cosf(m_Rad.lateY) * (-cosf(m_Rad.lateXZ)) * (m_Dist)+m_Target.z;

	// ビューマトリクス設定
	DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	DirectX::XMStoreFloat4x4(&m_ViewMatrix,
		DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_Position),
			DirectX::XMLoadFloat3(&m_Target), XMLoadFloat3(&up))
	);

	// プロジェクションマトリクス設定
	DirectX::XMStoreFloat4x4(&m_ProjectionMatrix,
		DirectX::XMMatrixOrthographicLH(100, 100, (float)CAM_NEAR, (float)CAM_FAR)
	);
}

void cLightBase::Update()
{
	// ライトの回転
	if (Input::GetKeyPress('L'))
	{
		if (Input::GetKeyPress('W'))
		{
			m_Rad.lateY += LIG_SPEED_X * SPEED * 3.1415f / 180.0f;
		}
		if (Input::GetKeyPress('S'))
		{
			m_Rad.lateY -= LIG_SPEED_X * SPEED * 3.1415f / 180.0f;
		}
		if (Input::GetKeyPress('D'))
		{
			m_Rad.lateXZ += LIG_SPEED_Y * SPEED * 3.1415f / 180.0f;
		}
		if (Input::GetKeyPress('A'))
		{
			m_Rad.lateXZ -= LIG_SPEED_Y * SPEED * 3.1415f / 180.0f;
		}
	}

	// 角度と距離・注視点からカメラ位置を計算
	m_Position.x = cosf(m_Rad.lateY) * sinf(m_Rad.lateXZ) * (m_Dist)+m_Target.x;
	m_Position.y = sinf(m_Rad.lateY) * (m_Dist)+m_Target.y;
	m_Position.z = cosf(m_Rad.lateY) * (-cosf(m_Rad.lateXZ)) * (m_Dist)+m_Target.z;

	m_Up = { 0,1,0 };
}

void cLightBase::Draw()
{
#ifdef _DEBUG
	ImGui::Begin("Light");
	{
		ImGui::SliderFloat("LightIntensity", &m_Power, 0, 10);
		ImGui::ColorEdit4("LightColor", (float*)&m_Color);
		ImGui::SliderFloat("Ambient", &m_Ambient, 0, 1);
	}
	ImGui::End();
#endif
}

DirectX::XMFLOAT4X4 cLightBase::GetLightViewProjectionMat()
{
	DirectX::XMMATRIX lvp;
	DirectX::XMMATRIX vM = DirectX::XMLoadFloat4x4(&m_ViewMatrix);
	DirectX::XMMATRIX pM = DirectX::XMLoadFloat4x4(&m_ProjectionMatrix);
	lvp = DirectX::XMMatrixMultiply(vM, pM);

	DirectX::XMFLOAT4X4 lvp4x4;
	DirectX::XMStoreFloat4x4(&lvp4x4, lvp);

	return lvp4x4;
}
