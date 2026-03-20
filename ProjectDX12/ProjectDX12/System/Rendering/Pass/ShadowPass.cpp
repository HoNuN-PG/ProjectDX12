
#include "GameObject/Camera/CameraBase.h"
#include "GameObject/Light/LightBase.h"
#include "Material/Materials/M_Shadow.h"
#include "Scene/SceneManager.h"
#include "System/Rendering/Pass/ShadowPass.h"
#include "System/Rendering/Volume/Blur.h""
#include "System/Rendering/GlobalResourceKey.h"
#include "System/Rendering/RenderingEngine.h"
#include "System/MyMath.h"

ShadowPass::ShadowPass()
{
	PassType = RenderingPass::RenderingPassType::Shadow;

	// シャドウマップテクスチャ設定
	ShadowMapsSize[TextureType::Near] = { 4096,4096 };
	ShadowMapsSize[TextureType::Middle] = { 2048 ,2048 };
	ShadowMapsSize[TextureType::Far] = { 1024,1024 };
	VSMShadowMapsSize[TextureType::Near] = { 4096,4096 };
	VSMShadowMapsSize[TextureType::Middle] = { 2048 ,2048 };
	VSMShadowMapsSize[TextureType::Far] = { 1024,1024 };
	ShadowMapsFormat = DXGI_FORMAT_R16G16_FLOAT;

	// カスケード設定
	CascadeAreas.resize(SHADOW_MAP_COUNT);
	CascadeAreas[0] = (50);
	CascadeAreas[1] = (250);
	CascadeAreas[2] = (CAM_FAR);

	pCamera = SceneManager::GetCurrentScene()->GetGameObject<CameraBase>();
}

void ShadowPass::Execute()
{
	static const float clearColor[4] = { 1, 1, 1, 1 };
	
	// シャドウマップの描画
	float nearDepth = CAM_NEAR;
	// 3枚のシャドウマップに描画
	for (int i = 0; i < SHADOW_MAP_COUNT; ++i)
	{
#if 0
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
#else 
		// 行列の計算
		DirectX::XMFLOAT4X4 LV = LightBase::GetLightViewMat();
		DirectX::XMFLOAT4X4 crop = CalcTexelSnappedCrop(nearDepth, i, ShadowMapsSize[i].x,LV);

		// ライトビュー行列とクロップ行列を乗算
		DirectX::XMMATRIX lvpc;
		DirectX::XMMATRIX m1 = DirectX::XMLoadFloat4x4(&LV);
		DirectX::XMMATRIX m2 = DirectX::XMLoadFloat4x4(&crop);
		lvpc = DirectX::XMMatrixMultiply(m1, m2);
		DirectX::XMFLOAT4X4 lvpc4x4;
		DirectX::XMStoreFloat4x4(&lvpc4x4, lvpc);
		DirectX::XMStoreFloat4x4(&lvpc4x4,
			DirectX::XMMatrixTranspose(
				DirectX::XMLoadFloat4x4(&lvpc4x4)
			)
		);
#endif
		// パラメータ設定
		M_ShadowMapsBase::CurrentShadowMapsNo = i;
		// シャドウマップ
		ShadowMapsParam = ShadowParam::ShadowMapsParam(lvpc4x4);
		pEngine->WriteGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowMaps1 + i, &ShadowMapsParam);
		// シャドウレシーバ
		ShadowReceiveParam.LVP[i] = lvpc4x4;

		// ターゲット化
		StagingShadowMaps[i]->SRV2RTV();

		// RTVの設定
		StagingShadowMaps[i]->Clear(clearColor);
		DSVs[i]->Clear();
		D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
		{
			StagingShadowMaps[i]->GetHandleRTV().hCPU,
		};
		SetRenderTarget(_countof(rtvs), rtvs, DSVs[i]->GetHandleDSV().hCPU);
		SetViewPort(StagingShadowMaps[i]->Width,StagingShadowMaps[i]->Height);

		// シャドウマップ描画
		for (int j = 0; j < RenderObjects.size(); ++j)
		{
			RenderObjects[j].obj.
				RenderingBase();
		}

		// リソース化
		StagingShadowMaps[i]->RTV2SRV();

		nearDepth = CascadeAreas[i];
	}
	SetViewPort(WINDOW_WIDTH, WINDOW_HEIGHT);

	// パラメータ設定
	ShadowReceiveParam.CascadeAreas = DirectX::XMFLOAT4( CascadeAreas[Near],CascadeAreas[Middle],CascadeAreas[Far],0 );
	pEngine->WriteGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowReciever,&ShadowReceiveParam);

	// ぼかし
	Gauss::ExecuteScreenGauss8D2(
		GaussIdx[TextureType::Near],
		{ VSMShadowMaps[TextureType::Near]->Width,VSMShadowMaps[TextureType::Near]->Height },
		StagingShadowMaps[TextureType::Near], 
		StagingVSMShadowMaps[TextureType::Near]
	);
	Gauss::ExecuteScreenGauss4D2(
		GaussIdx[TextureType::Middle],
		{ VSMShadowMaps[TextureType::Middle]->Width,VSMShadowMaps[TextureType::Middle]->Height },
		StagingShadowMaps[TextureType::Middle], 
		StagingVSMShadowMaps[TextureType::Middle]
	);
	Gauss::ExecuteScreenGauss2D1(
		GaussIdx[TextureType::Far],
		{ VSMShadowMaps[TextureType::Far]->Width,VSMShadowMaps[TextureType::Far]->Height },
		StagingShadowMaps[TextureType::Far], 
		StagingVSMShadowMaps[TextureType::Far]
	);

	// ステージングから読み取り専用テクスチャへコピー
	for (int i = 0; i <= TextureType::Far; ++i)
	{
		RenderingEngine::CopyTextureSRV(StagingShadowMaps[i].get()->GetHandleSRV().hCPU, ShadowMaps[i].get()->GetHandleSRV().hCPU);
		RenderingEngine::CopyTextureSRV(StagingVSMShadowMaps[i].get()->GetHandleSRV().hCPU, VSMShadowMaps[i].get()->GetHandleSRV().hCPU);
	}

	RenderObjects.clear();
}

void ShadowPass::Init(
	std::shared_ptr<DescriptorHeap> rtvHeap, 
	std::shared_ptr<DescriptorHeap> stagingHeap,
	std::shared_ptr<DescriptorHeap> srvHeap, 
	std::shared_ptr<DescriptorHeap> dsvHeap)
{
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.format = ShadowMapsFormat;
		desc.pRTVHeap = rtvHeap.get();
		desc.pSRVHeap = stagingHeap.get();
		desc.clearColor = 1;
		for (int i = 0; i <= TextureType::Far; ++i)
		{
			desc.width = ShadowMapsSize[i].x;
			desc.height = ShadowMapsSize[i].y;
			StagingShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
			desc.width = VSMShadowMapsSize[i].x;
			desc.height = VSMShadowMapsSize[i].y;
			StagingVSMShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
		}
		desc.pSRVHeap = srvHeap.get();
		for (int i = 0; i <= TextureType::Far; ++i)
		{
			desc.width		= ShadowMapsSize[i].x;
			desc.height		= ShadowMapsSize[i].y;
			ShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
			desc.width		= VSMShadowMapsSize[i].x;
			desc.height		= VSMShadowMapsSize[i].y;
			VSMShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
		}
	}
	// DSV
	{
		DepthStencil::Description desc = {};
		desc.pDSVHeap = dsvHeap.get();

		for (int i = 0; i <= TextureType::Far; ++i)
		{
			desc.width = ShadowMapsSize[i].x;
			desc.height = ShadowMapsSize[i].y;
			DSVs.push_back(std::make_unique<DepthStencil>(desc));
		}
	}

	// ガウスデータ作成
	GaussIdx[TextureType::Near] = Gauss::MakeGaussD2Data({ VSMShadowMaps[TextureType::Near]->Width,VSMShadowMaps[TextureType::Near]->Height });
	GaussIdx[TextureType::Middle] = Gauss::MakeGaussD2Data({ VSMShadowMaps[TextureType::Middle]->Width,VSMShadowMaps[TextureType::Middle]->Height });
	GaussIdx[TextureType::Far] = Gauss::MakeGaussD1Data({ VSMShadowMaps[TextureType::Far]->Width,VSMShadowMaps[TextureType::Far]->Height });
}

void ShadowPass::AddObj(GameObject& obj)
{
	RenderingEngine::RenderingInfo info = { obj };
	RenderObjects.push_back(info);
}

std::shared_ptr<RenderTarget> ShadowPass::GetTextureStaging(UINT idx)
{
	if (idx < StagingShadowMaps.size())
	{
		return StagingShadowMaps[idx];
	}
	else if (idx < (StagingShadowMaps.size() + StagingVSMShadowMaps.size()))
	{
		return StagingVSMShadowMaps[idx - StagingShadowMaps.size()];
	}
	return nullptr;
}

std::shared_ptr<RenderTarget> ShadowPass::GetTexture(UINT idx)
{
	if (idx < ShadowMaps.size())
	{
		return ShadowMaps[idx];
	}
	else if(idx < (ShadowMaps.size() + VSMShadowMaps.size()))
	{
		return VSMShadowMaps[idx - ShadowMaps.size()];
	}
	return nullptr;
}

DescriptorHeap::Handle ShadowPass::GetTextureStagingRTV(UINT idx)
{
	if (idx < StagingShadowMaps.size())
	{
		return StagingShadowMaps[idx]->GetHandleRTV();
	}
	else if (idx < (StagingShadowMaps.size() + StagingVSMShadowMaps.size()))
	{
		return StagingVSMShadowMaps[idx - StagingShadowMaps.size()]->GetHandleRTV();
	}
	return DescriptorHeap::Handle();
}

DescriptorHeap::Handle ShadowPass::GetTextureRTV(UINT idx)
{
	if (idx < ShadowMaps.size())
	{
		return ShadowMaps[idx]->GetHandleRTV();
	}
	else if (idx < (ShadowMaps.size() + VSMShadowMaps.size()))
	{
		return VSMShadowMaps[idx - ShadowMaps.size()]->GetHandleRTV();
	}
	return DescriptorHeap::Handle();
}

DescriptorHeap::Handle ShadowPass::GetTextureStagingSRV(UINT idx)
{
	if (idx < StagingShadowMaps.size())
	{
		return StagingShadowMaps[idx]->GetHandleSRV();
	}
	else if (idx < (ShadowMaps.size() + StagingVSMShadowMaps.size()))
	{
		return StagingVSMShadowMaps[idx - StagingShadowMaps.size()]->GetHandleSRV();
	}
	return DescriptorHeap::Handle();
}

DescriptorHeap::Handle ShadowPass::GetTextureSRV(UINT idx)
{
	if (idx < ShadowMaps.size())
	{
		return ShadowMaps[idx]->GetHandleSRV();
	}
	else if (idx < (ShadowMaps.size() + VSMShadowMaps.size()))
	{
		return VSMShadowMaps[idx - ShadowMaps.size()]->GetHandleSRV();
	}
	return DescriptorHeap::Handle();
}

DirectX::XMFLOAT4X4 ShadowPass::CalcCrop(
	float depth,
	int area,
	DirectX::XMFLOAT4X4 lvp
)
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

	vertex[0] = DXFL::Add(nearPos, DXFL::Add(nearUp, nearRight));
	vertex[1] = DXFL::Add(nearPos, DXFL::Add(nearUp, DXFL::Scale(nearRight, -1)));
	vertex[2] = DXFL::Add(nearPos, DXFL::Add(DXFL::Scale(nearUp, -1), nearRight));
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

DirectX::XMFLOAT4X4 ShadowPass::CalcTexelSnappedCrop(
	float depth,
	int area,
	float res,
	DirectX::XMFLOAT4X4 lv
)
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

	vertex[0] = DXFL::Add(nearPos, DXFL::Add(nearUp, nearRight));
	vertex[1] = DXFL::Add(nearPos, DXFL::Add(nearUp, DXFL::Scale(nearRight, -1)));
	vertex[2] = DXFL::Add(nearPos, DXFL::Add(DXFL::Scale(nearUp, -1), nearRight));
	vertex[3] = DXFL::Add(nearPos, DXFL::Add(DXFL::Scale(nearUp, -1), DXFL::Scale(nearRight, -1)));
	vertex[4] = DXFL::Add(farPos, DXFL::Add(farUp, farRight));
	vertex[5] = DXFL::Add(farPos, DXFL::Add(farUp, DXFL::Scale(farRight, -1)));
	vertex[6] = DXFL::Add(farPos, DXFL::Add(DXFL::Scale(farUp, -1), farRight));
	vertex[7] = DXFL::Add(farPos, DXFL::Add(DXFL::Scale(farUp, -1), DXFL::Scale(farRight, -1)));

	// 視錐台の中心と半径を求める
	DirectX::XMFLOAT3 center = DXFL::Scale(DXFL::Add(nearPos, farPos), 0.5f);
	float radius = 0.0f;
	for (int i = 0; i < 8; ++i)
	{
		float temp = DXFL::Magnitude(DXFL::Subtraction(vertex[i], center));
		radius = radius < temp ? temp : radius;
	}

	// 半径の調整
	radius *= 1.01f;

	// スナップ単位
	float unitsPerTexel = (radius * 2.0f) / res;

	radius = roundf(radius / unitsPerTexel) * unitsPerTexel;

	// ライトビュー変換
	{
		DirectX::XMVECTOR vec;
		vec = DirectX::XMLoadFloat3(&center);
		vec = DirectX::XMVector3Transform(vec, DirectX::XMLoadFloat4x4(&lv));
		DirectX::XMFLOAT3 v;
		DirectX::XMStoreFloat3(&v, vec);
		center = v;
	}

	// テクセルスナップ
	center.x = roundf(center.x / unitsPerTexel) * unitsPerTexel;
	center.y = roundf(center.y / unitsPerTexel) * unitsPerTexel;

	// 最大最小計算
	float minX, maxX, minY, maxY, minZ, maxZ;
	minX = center.x - radius;
	maxX = center.x + radius;
	minY = center.y - radius;
	maxY = center.y + radius;
	minZ = DXFL::Magnitude(DXFL::Subtraction(nearPos, pCamera->GetPosition()));
	maxZ = DXFL::Magnitude(DXFL::Subtraction(farPos, pCamera->GetPosition()));

	DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicOffCenterLH(
		minX,
		maxX,
		minY,
		maxY,
		-radius,
		radius
	);

	DirectX::XMFLOAT4X4 crop;
	DirectX::XMStoreFloat4x4(&crop, lightProj);
	return crop;
}