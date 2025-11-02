
#include "CameraBase.h"
#include "LightBase.h"

#include "MyMath.h"

#include "M_Shadow.h"

#include "SceneManager.h"

#include "ShadowPass.h"

#include "Blur.h"

#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

ShadowPass::ShadowPass()
{
	PassType = RenderingPass::RenderingPassType::Shadow;

	CascadeAreas.push_back(100);
	CascadeAreas.push_back(300);
	CascadeAreas.push_back(CAM_FAR);

	pCamera = SceneManager::GetCurrentScene()->GetGameObject<CameraBase>();

	for (int i = 0; i < TextureType::MAX; ++i)
	{
		GaussIdx[i] = -1;
	}

	ShadowMapsSize[TextureType::Near] = { 4096,4096 };
	ShadowMapsSize[TextureType::Middle] = {2048 ,2048 };
	ShadowMapsSize[TextureType::Far] = {1024,1024};
	VSMShadowMapsSize[TextureType::Near] = { 4096,4096 };
	VSMShadowMapsSize[TextureType::Middle] = { 2048 ,2048 };
	VSMShadowMapsSize[TextureType::Far] = { 1024,1024 };
	ShadowMapsFormat = DXGI_FORMAT_R16G16_FLOAT;
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

		// パラメータ設定
		M_ShadowMapsBase::CurrentShadowMapsNo = i;
		ShadowMapsParam = ShadowParam::ShadowMapsParam(lvpc4x4);
		Engine->WriteGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowMaps1 + i, &ShadowMapsParam);
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
		for (int j = 0; j < RenderObjects.size(); ++j)
		{
			RenderObjects[j].obj.
				ReuseRenderingBase();
		}

		// リソース化
		ShadowMaps[i]->ResourceBarrier(
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		nearDepth = CascadeAreas[i];
	}
	SetViewPort(WINDOW_WIDTH, WINDOW_HEIGHT);

	// パラメータ設定
	ShadowReceiveParam.CascadeAreas = DirectX::XMFLOAT4( CascadeAreas[Near],CascadeAreas[Middle],CascadeAreas[Far],0 );
	Engine->WriteGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowReciever,&ShadowReceiveParam);

	// ぼかし
	Gauss::ExecuteScreenGauss8D2(GaussIdx[TextureType::Near],{ VSMShadowMaps[TextureType::Near]->Width,VSMShadowMaps[TextureType::Near]->Height },
		ShadowMaps[TextureType::Near], VSMShadowMaps[TextureType::Near]);
	Gauss::ExecuteScreenGauss4D1(GaussIdx[TextureType::Middle], { VSMShadowMaps[TextureType::Middle]->Width,VSMShadowMaps[TextureType::Middle]->Height },
		ShadowMaps[TextureType::Middle], VSMShadowMaps[TextureType::Middle]);
	Gauss::ExecuteScreenGauss2D1(GaussIdx[TextureType::Far], { VSMShadowMaps[TextureType::Far]->Width,VSMShadowMaps[TextureType::Far]->Height },
		ShadowMaps[TextureType::Far], VSMShadowMaps[TextureType::Far]);

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
		desc.format = ShadowMapsFormat;
		desc.pRTVHeap = rtvHeap.get();
		desc.pSRVHeap = srvHeap.get();

		for (int i = 0; i < TextureType::Far + 1; ++i)
		{
			desc.width = ShadowMapsSize[i].x;
			desc.height = ShadowMapsSize[i].y;
			ShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
			desc.width = VSMShadowMapsSize[i].x;
			desc.height = VSMShadowMapsSize[i].y;
			VSMShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
		}
	}
	// DSV
	{
		DepthStencil::Description desc = {};
		desc.pDSVHeap = dsvHeap.get();

		for (int i = 0; i < TextureType::Far + 1; ++i)
		{
			desc.width = ShadowMapsSize[i].x;
			desc.height = ShadowMapsSize[i].y;
			DSVs.push_back(std::make_unique<DepthStencil>(desc));
		}
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
	else if(idx - (TextureType::MAX + 1) < VSMShadowMaps.size())
	{
		return VSMShadowMaps[idx - (TextureType::MAX + 1)];
	}
	return nullptr;
}

DescriptorHeap::Handle ShadowPass::GetTextureRTV(UINT idx)
{
	if (idx < ShadowMaps.size())
	{
		return ShadowMaps[idx]->GetHandleRTV();
	}
	else if (idx - (TextureType::MAX + 1) < VSMShadowMaps.size())
	{
		return VSMShadowMaps[idx - (TextureType::MAX + 1)]->GetHandleRTV();
	}
	return DescriptorHeap::Handle();
}

DescriptorHeap::Handle ShadowPass::GetTextureSRV(UINT idx)
{
	if (idx < ShadowMaps.size())
	{
		return ShadowMaps[idx]->GetHandleSRV();
	}
	else if (idx - (TextureType::MAX + 1) < VSMShadowMaps.size())
	{
		return VSMShadowMaps[idx - (TextureType::MAX + 1)]->GetHandleSRV();
	}
	return DescriptorHeap::Handle();
}
