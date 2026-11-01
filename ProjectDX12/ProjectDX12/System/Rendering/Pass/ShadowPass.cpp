
#include "GameObject/Camera/CameraBase.h"
#include "GameObject/Light/LightBase.h"
#include "Material/Materials/M_Shadow.h"
#include "System/Rendering/Pass/ShadowPass.h"
#include "System/Rendering/Volume/Blur.h"
#include "System/Rendering/GlobalResourceKey.h"
#include "System/Rendering/RenderingEngine.h"
#include "System/MyMath.h"

namespace
{
	// カスケードごとの設定
	struct CascadeSetting
	{
		UINT				smSize;			// シャドウマップの解像度
		UINT				vsmSize;		// VSM（ぼかし後）の解像度
		float				area;			// カスケードの奥の距離（初期値）
		Gauss::KernelType	kernel;			// ぼかしのカーネルサイズ
		UINT				downSample;		// ぼかしのダウンサンプリング
	};
	constexpr CascadeSetting CascadeSettings[] =
	{
		{ 4096, 4096, 50.0f,	Gauss::KernelType::Tap8, 2 },	// Near
		{ 2048, 2048, 250.0f,	Gauss::KernelType::Tap4, 2 },	// Middle
		{ 1024, 1024, CAM_FAR,	Gauss::KernelType::Tap2, 1 },	// Far
	};
	static_assert(_countof(CascadeSettings) == ShadowParam::CASCADE_COUNT, "カスケード設定の数が一致していない");
}

ShadowPass::ShadowPass(RenderingEngine& engine)
	:
	RenderingPass(engine)
{
	PassType = RenderingPass::RenderingPassType::Shadow;
	PassFormats.push_back(DXGI_FORMAT_R16G16_FLOAT);

	// カスケード設定
	for (UINT i = 0; i < ShadowParam::CASCADE_COUNT; ++i)
	{
		CascadeAreas[i] = CascadeSettings[i].area;
	}
}

void ShadowPass::Execute()
{
	static const float clearColor[4] = { 1, 1, 1, 1 };

	// デバッグ用にカメラを切り替えても、影はメインカメラの視錐台を基準にする
	CameraBase* camera = pEngine->GetMainCamera();
	assert(camera);
	LightBase* light = pEngine->GetLight();
	assert(light);

	// シャドウマップの描画
	float nearDepth = CAM_NEAR;
	for (UINT i = 0; i < ShadowParam::CASCADE_COUNT; ++i)
	{
#if 0
		// クロップ行列の計算
		DirectX::XMFLOAT4X4 LVP = light->GetLightViewProjectionMat();
		DirectX::XMFLOAT4X4 crop = CalcCrop(*camera, nearDepth, i, LVP);

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
		DirectX::XMFLOAT4X4 LV = light->GetLightViewMat();
		DirectX::XMFLOAT4X4 crop = CalcTexelSnappedCrop(*camera, nearDepth, i, static_cast<float>(CascadeSettings[i].smSize), LV);

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
		CurrentCascade = i;
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
	for (UINT i = 0; i < ShadowParam::CASCADE_COUNT; ++i)
	{
		pEngine->GetGauss()->Execute(GaussIdx[i], StagingShadowMaps[i].get(), StagingVSMShadowMaps[i].get());
	}

	RenderObjects.clear();
}

void ShadowPass::Init(
	DescriptorHeap* rtvHeap,
	DescriptorHeap* stagingHeap,
	DescriptorHeap* srvHeap,
	DescriptorHeap* dsvHeap)
{
	for (UINT i = 0; i < ShadowParam::CASCADE_COUNT; ++i)
	{
		const CascadeSetting& setting = CascadeSettings[i];

		// RTV
		{
			RenderTarget::Description desc = {};
			desc.format = DXGI_FORMAT_R16G16_FLOAT;
			desc.pRTVHeap = rtvHeap;
			desc.pSRVHeap = stagingHeap;
			desc.clearColor = 1;
			desc.width = setting.smSize;
			desc.height = setting.smSize;
			StagingShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
			desc.width = setting.vsmSize;
			desc.height = setting.vsmSize;
			StagingVSMShadowMaps.push_back(std::make_shared<RenderTarget>(desc));
		}
		// SRV（ステージングのリソースは作り直さないため初期化時に一度だけコピーする）
		{
			ShadowMapSRVs.push_back(srvHeap->Allocate());
			RenderingEngine::CopyTexture(StagingShadowMaps[i]->GetHandleSRV().hCPU, ShadowMapSRVs[i].hCPU);
			VSMShadowMapSRVs.push_back(srvHeap->Allocate());
			RenderingEngine::CopyTexture(StagingVSMShadowMaps[i]->GetHandleSRV().hCPU, VSMShadowMapSRVs[i].hCPU);
		}
		// DSV
		{
			DepthStencil::Description desc = {};
			desc.pDSVHeap = dsvHeap;
			desc.width = setting.smSize;
			desc.height = setting.smSize;
			DSVs.push_back(std::make_unique<DepthStencil>(desc));
		}
		// ガウスデータ作成
		GaussIdx[i] = pEngine->GetGauss()->MakeGaussData(
			{ StagingVSMShadowMaps[i]->Width, StagingVSMShadowMaps[i]->Height },
			setting.kernel,
			setting.downSample
		);
	}
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
	else if (idx < MAX)
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
	std::shared_ptr<RenderTarget> texture = GetTextureStaging(idx);
	return texture ? texture->GetHandleRTV() : DescriptorHeap::Handle();
}

DescriptorHeap::Handle ShadowPass::GetTextureRTV(UINT idx)
{
	return GetTextureStagingRTV(idx);
}

DescriptorHeap::Handle ShadowPass::GetTextureStagingSRV(UINT idx)
{
	std::shared_ptr<RenderTarget> texture = GetTextureStaging(idx);
	return texture ? texture->GetHandleSRV() : DescriptorHeap::Handle();
}

DescriptorHeap::Handle ShadowPass::GetTextureSRV(UINT idx)
{
	if (idx < NearVSM)
	{
		return ShadowMapSRVs[idx];
	}
	else if (idx < MAX)
	{
		return VSMShadowMapSRVs[idx - NearVSM];
	}
	return DescriptorHeap::Handle();
}

ShadowPass::FrustumSlice ShadowPass::CalcFrustumSlice(CameraBase& camera, float nearDepth, float farDepth)
{
	// 手前の距離
	float nearY = tanf(CameraBase::GetViewAngle() * 0.5f) * nearDepth;
	float nearX = nearY * CameraBase::GetAspect();

	// 奥の距離
	float farY = tanf(CameraBase::GetViewAngle() * 0.5f) * farDepth;
	float farX = farY * CameraBase::GetAspect();

	// 手前の面の中心位置
	DirectX::XMFLOAT3 nearPos = DXFL::Add(camera.GetPosition(), DXFL::Scale(camera.GetForward(), nearDepth));
	// 奥の面の中心位置
	DirectX::XMFLOAT3 farPos = DXFL::Add(camera.GetPosition(), DXFL::Scale(camera.GetForward(), farDepth));

	// ８頂点の計算
	FrustumSlice slice;

	DirectX::XMFLOAT3 up = camera.GetUp();
	DirectX::XMFLOAT3 nearUp = DXFL::Scale(up, nearY);
	DirectX::XMFLOAT3 farUp = DXFL::Scale(up, farY);

	DirectX::XMFLOAT3 right = camera.GetRight();
	DirectX::XMFLOAT3 nearRight = DXFL::Scale(right, nearX);
	DirectX::XMFLOAT3 farRight = DXFL::Scale(right, farX);

	slice.vertex[0] = DXFL::Add(nearPos, DXFL::Add(nearUp, nearRight));
	slice.vertex[1] = DXFL::Add(nearPos, DXFL::Add(nearUp, DXFL::Scale(nearRight, -1)));
	slice.vertex[2] = DXFL::Add(nearPos, DXFL::Add(DXFL::Scale(nearUp, -1), nearRight));
	slice.vertex[3] = DXFL::Add(nearPos, DXFL::Add(DXFL::Scale(nearUp, -1), DXFL::Scale(nearRight, -1)));
	slice.vertex[4] = DXFL::Add(farPos, DXFL::Add(farUp, farRight));
	slice.vertex[5] = DXFL::Add(farPos, DXFL::Add(farUp, DXFL::Scale(farRight, -1)));
	slice.vertex[6] = DXFL::Add(farPos, DXFL::Add(DXFL::Scale(farUp, -1), farRight));
	slice.vertex[7] = DXFL::Add(farPos, DXFL::Add(DXFL::Scale(farUp, -1), DXFL::Scale(farRight, -1)));

	slice.center = DXFL::Scale(DXFL::Add(nearPos, farPos), 0.5f);
	return slice;
}

DirectX::XMFLOAT4X4 ShadowPass::CalcCrop(
	CameraBase& camera,
	float depth,
	int area,
	DirectX::XMFLOAT4X4 lvp
)
{
	FrustumSlice slice = CalcFrustumSlice(camera, depth, CascadeAreas[area]);
	DirectX::XMFLOAT3* vertex = slice.vertex;

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
	CameraBase& camera,
	float depth,
	int area,
	float res,
	DirectX::XMFLOAT4X4 lv
)
{
	FrustumSlice slice = CalcFrustumSlice(camera, depth, CascadeAreas[area]);

	// 視錐台の中心と半径を求める
	DirectX::XMFLOAT3 center = slice.center;
	float radius = 0.0f;
	for (int i = 0; i < 8; ++i)
	{
		float temp = DXFL::Magnitude(DXFL::Subtraction(slice.vertex[i], center));
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
