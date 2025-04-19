
#include "ConstantWVP.h"

#include "Game.h"
#include "CameraBase.h"

void * cConstantWVP::Calc3DMatrix(DirectX::XMFLOAT3 Pos, DirectX::XMFLOAT3 Rot, DirectX::XMFLOAT3 Scale)
{
	sConstantWVP wvp;

	// ワールドマトマトリクス設定
	DirectX::XMMATRIX scale, rot, trans;
	scale = DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);
	rot = DirectX::XMMatrixRotationRollPitchYaw(Rot.x, Rot.y, Rot.z);
	trans = DirectX::XMMatrixTranslation(Pos.x, Pos.y, Pos.z);
	DirectX::XMMATRIX world = scale * rot * trans;
	world = DirectX::XMMatrixTranspose(world);

	DirectX::XMStoreFloat4x4(&(wvp.world), world);

	// ビューマトリクス設定
	DirectX::XMFLOAT3 pos = { cCameraDebug::m_MainPos.x,cCameraDebug::m_MainPos.y,cCameraDebug::m_MainPos.z };
	DirectX::XMFLOAT3 target = { cCameraDebug::m_MainTarget.x,cCameraDebug::m_MainTarget.y,cCameraDebug::m_MainTarget.z };
	DirectX::XMFLOAT3 up = { cCameraDebug::m_MainUp.x,cCameraDebug::m_MainUp.y,cCameraDebug::m_MainUp.z };
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&pos),
		DirectX::XMLoadFloat3(&target), XMLoadFloat3(&up));
	view = DirectX::XMMatrixTranspose(view);

	DirectX::XMStoreFloat4x4(&(wvp.view), view);

	// プロジェクションマトリクス設定
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60),
		(float)WINDOW_WIDTH / WINDOW_HEIGHT, (float)CAM_NEAR, (float)CAM_FAR);
	proj = DirectX::XMMatrixTranspose(proj);

	DirectX::XMStoreFloat4x4(&(wvp.proj), proj);

	return &wvp;
}

void * cConstantWVP::Calc2DMatrix(DirectX::XMFLOAT3 Pos, DirectX::XMFLOAT3 Rot, DirectX::XMFLOAT3 Scale)
{
	sConstantWVP wvp;

	DirectX::XMMATRIX scale, rot, trans;
	scale = DirectX::XMMatrixScaling(Scale.x, Scale.y, 0);
	rot = DirectX::XMMatrixRotationRollPitchYaw(Rot.x, Rot.y, Rot.z);
	trans = DirectX::XMMatrixTranslation(Pos.x, Pos.y, 0);
	DirectX::XMMATRIX world = scale * rot * trans;
	world = DirectX::XMMatrixTranspose(world);

	DirectX::XMStoreFloat4x4(&(wvp.world), world);

	DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
	view = DirectX::XMMatrixTranspose(view);

	DirectX::XMStoreFloat4x4(&(wvp.view), view);

	DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, WINDOW_WIDTH, -WINDOW_HEIGHT, 0.0f, -1.0f, 1.0f);
	proj = DirectX::XMMatrixTranspose(proj);

	DirectX::XMStoreFloat4x4(&(wvp.proj), proj);

	return &wvp;
}

DirectX::XMFLOAT4X4 cConstantWVP::CalcInversVPMatric()
{
	// ビューマトリクス
	DirectX::XMFLOAT3 pos = { cCameraDebug::m_MainPos.x,cCameraDebug::m_MainPos.y,cCameraDebug::m_MainPos.z };
	DirectX::XMFLOAT3 target = { cCameraDebug::m_MainTarget.x,cCameraDebug::m_MainTarget.y,cCameraDebug::m_MainTarget.z };
	DirectX::XMFLOAT3 up = { cCameraDebug::m_MainUp.x,cCameraDebug::m_MainUp.y,cCameraDebug::m_MainUp.z };
	DirectX::XMMATRIX viewM = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&pos),
		DirectX::XMLoadFloat3(&target), XMLoadFloat3(&up));
	viewM = DirectX::XMMatrixTranspose(viewM);

	// プロジェクションマトリクス
	DirectX::XMMATRIX projM = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60),
		(float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.01f, 1000.0f);
	projM = DirectX::XMMatrixTranspose(projM);

	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
	DirectX::XMStoreFloat4x4(&view, viewM);
	DirectX::XMStoreFloat4x4(&proj, projM);

	// 逆行列
	DirectX::XMFLOAT4X4 vpInv;
	DirectX::XMStoreFloat4x4(&vpInv, 
		DirectX::XMMatrixTranspose(
			DirectX::XMMatrixInverse(nullptr,DirectX::XMLoadFloat4x4(&view) * DirectX::XMLoadFloat4x4(&proj)))
	);

	return vpInv;
}
