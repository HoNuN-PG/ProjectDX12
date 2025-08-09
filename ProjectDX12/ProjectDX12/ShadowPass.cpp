
#include "ShadowPass.h"

#include "CameraBase.h"
#include "LightBase.h"

#include "MyMath.h"
#include "SceneManager.h"
#include "RenderingEngine.h"

#include "GlobalResourceKey.h"

ShadowPass::ShadowPass()
{
	PassType = RenderingPass::RenderingPassType::Shadow;
	CascadeAreas.push_back(100);
	CascadeAreas.push_back(500);
	CascadeAreas.push_back(1000);
	pCamera = SceneManager::GetCurrentScene()->GetGameObject<CameraBase>();
}

void ShadowPass::Execute()
{
	static const float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1 };
	
	// シャドウマップの描画
	float nearDepth = CAM_NEAR;
	for (int i = 0; i < CascadeAreas.size(); ++i) // 3枚のシャドウマップに描画
	{
		// クロップ行列の計算
		DirectX::XMFLOAT4X4 LVP = LightBase::GetLightViewProjectionMat();
		DirectX::XMFLOAT4X4 crop = CalcCrop(nearDepth, i, LVP);

		// ライトビュープロジェクション行列とクロップ行列を乗算
		DirectX::XMMATRIX lvpc;
		DirectX::XMMATRIX m1 = DirectX::XMLoadFloat4x4(&LVP);
		DirectX::XMMATRIX m2 = DirectX::XMLoadFloat4x4(&crop);
		lvpc = DirectX::XMMatrixMultiply(m1, m2);
		DirectX::XMFLOAT4X4 lvpc4x4;
		DirectX::XMStoreFloat4x4(&lvpc4x4, lvpc);
		DirectX::XMStoreFloat4x4(&lvpc4x4,
			DirectX::XMMatrixTranspose(
				DirectX::XMLoadFloat4x4(&lvpc4x4)
			)
		);
		ShadowMapsParam = ShadowParam::ShadowMapsParam(lvpc4x4);
		Engine->WriteGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowMaps, &ShadowMapsParam);
		ShadowReceiveParam.LVP[i] = lvpc4x4;

		// ターゲット化
		ShadowMaps[i]->ResourceBarrier(
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// RTVの設定
		ShadowMaps[i]->Clear(clearColor);
		DSVs[i]->Clear();
		D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
			ShadowMaps[i]->GetHandleRTV().hCPU,
		};
		SetRenderTarget(_countof(rtvs), rtvs, DSVs[i]->GetHandleDSV().hCPU);
		SetViewPort(ShadowMaps[i]->Width,ShadowMaps[i]->Height);

		// シャドウマップ描画
		for (int j = 0; j < RenderObjects.size(); ++j)
		{
			RenderObjects[j].obj.
				RenderingBase();
		}

		// リソース化
		ShadowMaps[i]->ResourceBarrier(
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		nearDepth = CascadeAreas[i];
		break;
	}
	Engine->WriteGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowReceive,&ShadowReceiveParam);

	SetViewPort(WINDOW_WIDTH,WINDOW_HEIGHT);

	RenderObjects.clear();
}

DirectX::XMFLOAT4X4 ShadowPass::CalcCrop(
	float depth,
	int area,
	DirectX::XMFLOAT4X4 lvp)
{
	// 手前の距離
	float nearY = tanf(CameraBase::GetViewAngle() * 0.5f) * depth;
	float nearX = nearY * CameraBase::GetAspect();

	// 奥の距離
	float farY = tanf(CameraBase::GetViewAngle() * 0.5f) * CascadeAreas[area];
	float farX = farY * CameraBase::GetAspect();

	// 手前の面の中心位置
	DirectX::XMFLOAT3 nearPos = DXFL::Add(pCamera->GetPosition(), DXFL::Scale(pCamera->GetForward(), depth));
	// 奥の面の中心位置
	DirectX::XMFLOAT3 farPos = DXFL::Add(pCamera->GetPosition(), DXFL::Scale(pCamera->GetForward(), CascadeAreas[area]));

	// ８頂点の計算
	DirectX::XMFLOAT3 vertex[8];

	DirectX::XMFLOAT3 up = CameraBase::m_MainUp;
	DirectX::XMFLOAT3 nearUp = DXFL::Scale(up, nearY);
	DirectX::XMFLOAT3 farUp = DXFL::Scale(up, farY);

	DirectX::XMFLOAT3 right = pCamera->GetRight();
	DirectX::XMFLOAT3 nearRight = DXFL::Scale(right, nearX);
	DirectX::XMFLOAT3 farRight = DXFL::Scale(right, farX);

	vertex[0] = DXFL::Add(nearPos,DXFL::Add(nearUp,nearRight));
	vertex[1] = DXFL::Add(nearPos, DXFL::Add(nearUp, DXFL::Scale(nearRight,-1)));
	vertex[2] = DXFL::Add(nearPos, DXFL::Add(DXFL::Scale(nearUp,-1), nearRight));
	vertex[3] = DXFL::Add(nearPos, DXFL::Add(DXFL::Scale(nearUp, -1), DXFL::Scale(nearRight, -1)));
	vertex[4] = DXFL::Add(farPos, DXFL::Add(farUp, farRight));
	vertex[5] = DXFL::Add(farPos, DXFL::Add(farUp, DXFL::Scale(farRight, -1)));
	vertex[6] = DXFL::Add(farPos, DXFL::Add(DXFL::Scale(farUp, -1), farRight));
	vertex[7] = DXFL::Add(farPos, DXFL::Add(DXFL::Scale(farUp, -1), DXFL::Scale(farRight, -1)));

	// ライトビュープロジェクション変換
	for (int i = 0; i < 8; ++i)
	{
		DirectX::XMVECTOR vec;
		vec = DirectX::XMLoadFloat3(&vertex[i]);
		vec = DirectX::XMVector3Transform(vec, DirectX::XMLoadFloat4x4(&lvp));
		DirectX::XMFLOAT3 v;
		DirectX::XMStoreFloat3(&v, vec);
		vertex[i] = v;
	}
	// 最大値と最小値の計算
	float maxX, minX, maxY, minY;
	maxX = -1000; minX = 1000; maxY = -1000; minY = 1000;
	for (int i = 0; i < 8; ++i)
	{
		maxX = (std::max)(maxX, vertex[i].x);
		minX = (std::min)(minX, vertex[i].x);
		maxY = (std::max)(maxY, vertex[i].y);
		minY = (std::min)(minY, vertex[i].y);
	}

	DirectX::XMFLOAT4X4 crop;
	DirectX::XMStoreFloat4x4(&crop, DirectX::XMMatrixIdentity());
	float xScale = 2.0f / (maxX - minX);
	float yScale = 2.0f / (maxY - minY);
	float xOffset = (maxX + minX) * (-0.5f) * xScale;
	float yOffset = (maxY + minY) * (-0.5f) * yScale;
	crop.m[0][0] = xScale;
	crop.m[1][1] = yScale;
	crop.m[3][0] = xOffset;
	crop.m[3][1] = yOffset;
	return crop;
}

void ShadowPass::Init(
	std::shared_ptr<DescriptorHeap> rtvHeap, 
	std::shared_ptr<DescriptorHeap> srvHeap, 
	std::shared_ptr<DescriptorHeap> dsvHeap)
{
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.format = DXGI_FORMAT_R16G16_FLOAT;
		desc.pRTVHeap = rtvHeap.get();
		desc.pSRVHeap = srvHeap.get();

		desc.width = WINDOW_WIDTH * 2;
		desc.height = WINDOW_HEIGHT * 2;
		ShadowMaps.push_back(std::make_shared<RenderTarget>(desc));

		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		ShadowMaps.push_back(std::make_shared<RenderTarget>(desc));

		desc.width = WINDOW_WIDTH / 2;
		desc.height = WINDOW_HEIGHT / 2;
		ShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
	}
	// DSV
	{
		DepthStencil::Description desc = {};
		desc.pDSVHeap = dsvHeap.get();

		desc.width = WINDOW_WIDTH * 2;
		desc.height = WINDOW_HEIGHT * 2;
		DSVs.push_back(std::make_unique<DepthStencil>(desc));

		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		DSVs.push_back(std::make_unique<DepthStencil>(desc));

		desc.width = WINDOW_WIDTH / 2;
		desc.height = WINDOW_HEIGHT / 2;
		DSVs.push_back(std::make_unique<DepthStencil>(desc));
	}
}

void ShadowPass::AddObj(GameObject& obj)
{
	RenderingEngine::RenderingInfo info = { obj };
	RenderObjects.push_back(info);
}

std::shared_ptr<RenderTarget> ShadowPass::GetTexture(UINT idx)
{
	if (idx < ShadowMaps.size())
	{
		return ShadowMaps[idx];
	}
	return nullptr;
}

DescriptorHeap::Handle ShadowPass::GetTextureRTV(UINT idx)
{
	if (idx < ShadowMaps.size())
	{
		return ShadowMaps[idx]->GetHandleRTV();
	}
	return DescriptorHeap::Handle();
}

DescriptorHeap::Handle ShadowPass::GetTextureSRV(UINT idx)
{
	if (idx < ShadowMaps.size())
	{
		return ShadowMaps[idx]->GetHandleSRV();
	}
	return DescriptorHeap::Handle();
}
