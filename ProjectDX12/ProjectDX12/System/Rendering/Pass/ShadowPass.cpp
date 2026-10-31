
#include "GameObject/Camera/CameraBase.h"
#include "GameObject/Light/LightBase.h"
#include "Material/Materials/M_Shadow.h"
#include "Scene/SceneManager.h"
#include "System/Rendering/Pass/ShadowPass.h"
#include "System/Rendering/Volume/Blur.h"
#include "System/Rendering/GlobalResourceKey.h"
#include "System/Rendering/RenderingEngine.h"
#include "System/MyMath.h"

ShadowPass::ShadowPass(RenderingEngine& engine)
	:
	RenderingPass(engine)
{
	PassType = RenderingPass::RenderingPassType::Shadow;

	// シャドウマップテクスチャ設定
	ShadowMapsSize[TextureType::Near] = { 4096,4096 };
	ShadowMapsSize[TextureType::Middle] = { 2048 ,2048 };
	ShadowMapsSize[TextureType::Far] = { 1024,1024 };
	VSMShadowMapsSize[TextureType::Near] = { 4096,4096 };
	VSMShadowMapsSize[TextureType::Middle] = { 2048 ,2048 };
	VSMShadowMapsSize[TextureType::Far] = { 1024,1024 };
	for (int i = 0; i < TextureType::MAX; ++i)
	{
		PassFormats.push_back(DXGI_FORMAT_R16G16_FLOAT);
	}

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
	pEngine->GetGauss()->ExecuteScreenGauss8D2(
		GaussIdx[TextureType::Near],
		{ StagingVSMShadowMaps[TextureType::Near]->Width,StagingVSMShadowMaps[TextureType::Near]->Height },
		StagingShadowMaps[TextureType::Near].get(), 
		StagingVSMShadowMaps[TextureType::Near].get()
	);
	pEngine->GetGauss()->ExecuteScreenGauss4D2(
		GaussIdx[TextureType::Middle],
		{ StagingVSMShadowMaps[TextureType::Middle]->Width,StagingVSMShadowMaps[TextureType::Middle]->Height },
		StagingShadowMaps[TextureType::Middle].get(), 
		StagingVSMShadowMaps[TextureType::Middle].get()
	);
	pEngine->GetGauss()->ExecuteScreenGauss2D1(
		GaussIdx[TextureType::Far],
		{ StagingVSMShadowMaps[TextureType::Far]->Width,StagingVSMShadowMaps[TextureType::Far]->Height },
		StagingShadowMaps[TextureType::Far].get(), 
		StagingVSMShadowMaps[TextureType::Far].get()
	);

	RenderObjects.clear();
}

void ShadowPass::Init(
	DescriptorHeap* rtvHeap, 
	DescriptorHeap* stagingHeap,
	DescriptorHeap* srvHeap, 
	DescriptorHeap* dsvHeap)
{
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.format = DXGI_FORMAT_R16G16_FLOAT;
		desc.pRTVHeap = rtvHeap;
		desc.pSRVHeap = stagingHeap;
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
	}
	// SRV（ステージングのリソースは作り直さないため初期化時に一度だけコピーする）
	{
		for (int i = 0; i <= TextureType::Far; ++i)
		{
			ShadowMapSRVs.push_back(srvHeap->Allocate());
			RenderingEngine::CopyTexture(StagingShadowMaps[i]->GetHandleSRV().hCPU, ShadowMapSRVs[i].hCPU);
			VSMShadowMapSRVs.push_back(srvHeap->Allocate());
			RenderingEngine::CopyTexture(StagingVSMShadowMaps[i]->GetHandleSRV().hCPU, VSMShadowMapSRVs[i].hCPU);
		}
	}
	// DSV
	{
		DepthStencil::Description desc = {};
		desc.pDSVHeap = dsvHeap;

		for (int i = 0; i <= TextureType::Far; ++i)
		{
			desc.width = ShadowMapsSize[i].x;
			desc.height = ShadowMapsSize[i].y;
			DSVs.push_back(std::make_unique<DepthStencil>(desc));
		}
	}

	// ガウスデータ作成
	GaussIdx[TextureType::Near] = pEngine->GetGauss()->MakeGaussD2Data({ StagingVSMShadowMaps[TextureType::Near]->Width,StagingVSMShadowMaps[TextureType::Near]->Height });
	GaussIdx[TextureType::Middle] = pEngine->GetGauss()->MakeGaussD2Data({ StagingVSMShadowMaps[TextureType::Middle]->Width,StagingVSMShadowMaps[TextureType::Middle]->Height });
	GaussIdx[TextureType::Far] = pEngine->GetGauss()->MakeGaussD1Data({ StagingVSMShadowMaps[TextureType::Far]->Width,StagingVSMShadowMaps[TextureType::Far]->Height });
}

void ShadowPass::AddObj(GameObject& obj)
{
	RenderingEngine::RenderingInfo info = { obj };
	RenderObjects.push_back(info);
}

std::shared_ptr<RenderTarget> ShadowPass::GetTextureStaging(UINT idx)
{
	if (idx < NearVSM)
	{
		return StagingShadowMaps[idx];
	}
	else if (idx <= FarVSM)
	{
		return StagingVSMShadowMaps[idx - NearVSM];
	}
	return nullptr;
}

std::shared_ptr<RenderTarget> ShadowPass::GetTexture(UINT idx)
{
	// 読み取り用の別リソースは持たない（SRVだけをシェーダー可視ヒープにコピーしている）
	return GetTextureStaging(idx);
}

DescriptorHeap::Handle ShadowPass::GetTextureStagingRTV(UINT idx)
{
	if (idx < NearVSM)
	{
		return StagingShadowMaps[idx]->GetHandleRTV();
	}
	else if (idx <= FarVSM)
	{
		return StagingVSMShadowMaps[idx - NearVSM]->GetHandleRTV();
	}
	return DescriptorHeap::Handle();
}

DescriptorHeap::Handle ShadowPass::GetTextureRTV(UINT idx)
{
	return GetTextureStagingRTV(idx);
}

DescriptorHeap::Handle ShadowPass::GetTextureStagingSRV(UINT idx)
{
	if (idx < NearVSM)
	{
		return StagingShadowMaps[idx]->GetHandleSRV();
	}
	else if (idx <= FarVSM)
	{
		return StagingVSMShadowMaps[idx - NearVSM]->GetHandleSRV();
	}
	return DescriptorHeap::Handle();
}

DescriptorHeap::Handle ShadowPass::GetTextureSRV(UINT idx)
{
	if (idx < NearVSM)
	{
		return ShadowMapSRVs[idx];
	}
	else if (idx <= FarVSM)
	{
		return VSMShadowMapSRVs[idx - NearVSM];
	}
	return DescriptorHeap::Handle();
}

DirectX::XMFLOAT4X4 ShadowPass::CalcCrop(
	float depth,
	int area,
	DirectX::XMFLOAT4X4 lvp
)
{
	std::shared_ptr<CameraBase> camera = pCamera.lock();
	assert(camera);
	
	// 手前の距離
	float nearY = tanf(CameraBase::GetViewAngle() * 0.5f) * depth;
	float nearX = nearY * CameraBase::GetAspect();

	// 奥の距離
	float farY = tanf(CameraBase::GetViewAngle() * 0.5f) * CascadeAreas[area];
	float farX = farY * CameraBase::GetAspect();

	// 手前の面の中心位置
	DirectX::XMFLOAT3 nearPos = DXFL::Add(camera->GetPosition(), DXFL::Scale(camera->GetForward(), depth));
	// 奥の面の中心位置
	DirectX::XMFLOAT3 farPos = DXFL::Add(camera->GetPosition(), DXFL::Scale(camera->GetForward(), CascadeAreas[area]));

	// ８頂点の計算
	DirectX::XMFLOAT3 vertex[8];

	DirectX::XMFLOAT3 up = CameraBase::m_MainUp;
	DirectX::XMFLOAT3 nearUp = DXFL::Scale(up, nearY);
	DirectX::XMFLOAT3 farUp = DXFL::Scale(up, farY);

	DirectX::XMFLOAT3 right = camera->GetRight();
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
	std::shared_ptr<CameraBase> camera = pCamera.lock();
	assert(camera);	
	
	// 手前の距離
	float nearY = tanf(CameraBase::GetViewAngle() * 0.5f) * depth;
	float nearX = nearY * CameraBase::GetAspect();

	// 奥の距離
	float farY = tanf(CameraBase::GetViewAngle() * 0.5f) * CascadeAreas[area];
	float farX = farY * CameraBase::GetAspect();

	// 手前の面の中心位置
	DirectX::XMFLOAT3 nearPos = DXFL::Add(camera->GetPosition(), DXFL::Scale(camera->GetForward(), depth));
	// 奥の面の中心位置
	DirectX::XMFLOAT3 farPos = DXFL::Add(camera->GetPosition(), DXFL::Scale(camera->GetForward(), CascadeAreas[area]));

	// ８頂点の計算
	DirectX::XMFLOAT3 vertex[8];

	DirectX::XMFLOAT3 up = CameraBase::m_MainUp;
	DirectX::XMFLOAT3 nearUp = DXFL::Scale(up, nearY);
	DirectX::XMFLOAT3 farUp = DXFL::Scale(up, farY);

	DirectX::XMFLOAT3 right = camera->GetRight();
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
	// 奥行きはライトビュー空間での中心を基準にする
	// 範囲外のキャスターも影を落とせるよう手前側はライト位置まで広げる
	minZ = 0.0f;
	maxZ = (std::max)(center.z + radius, minZ + 1.0f);

	DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicOffCenterLH(
		minX,
		maxX,
		minY,
		maxY,
		minZ,
		maxZ
	);

	DirectX::XMFLOAT4X4 crop;
	DirectX::XMStoreFloat4x4(&crop, lightProj);
	return crop;
}