#ifndef ___SHADOW_PASS_H___
#define ___SHADOW_PASS_H___

#include <memory>
#include <vector>

#include "Material/Materials/M_Shadow.h"
#include "System/Rendering/Pass/RenderingPass.h"
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Texture/DepthStencil.h"
#include "System/Rendering/Texture/RenderTarget.h"
#include "System/Rendering/RenderingEngine.h"

class CameraBase;

class ShadowPass : public RenderingPass
{
public:

	enum TextureType
	{
		Near = 0,
		Middle,
		Far,
		NearVSM,
		MiddleVSM,
		FarVSM,

		MAX
	};
	static_assert(TextureType::NearVSM == ShadowParam::CASCADE_COUNT, "シャドウマップの種類とカスケード数が一致していない");

public:

	explicit ShadowPass(RenderingEngine& engine);
	~ShadowPass() {};
	void Execute() override;

public:

	virtual void Init(
		DescriptorHeap* rtvHeap,
		DescriptorHeap* stagingHeap,
		DescriptorHeap* srvHeap,
		DescriptorHeap* dsvHeap
	) override;
	void AddObj(GameObject& obj) override;

public:

	virtual std::shared_ptr<RenderTarget> GetTextureStaging(UINT idx) override;
	virtual std::shared_ptr<RenderTarget> GetTexture(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureStagingRTV(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureRTV(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureStagingSRV(UINT idx) override;
	virtual DescriptorHeap::Handle GetTextureSRV(UINT idx) override;

private:

	std::vector<RenderingEngine::RenderingInfo> RenderObjects;

private:

	// カメラの視錐台のうち nearDepth～farDepth の範囲
	struct FrustumSlice
	{
		DirectX::XMFLOAT3 vertex[8];
		DirectX::XMFLOAT3 center;	// 手前の面と奥の面の中心の中点
	};
	FrustumSlice CalcFrustumSlice(CameraBase& camera, float nearDepth, float farDepth);

	DirectX::XMFLOAT4X4 CalcCrop(
		CameraBase& camera,
		float depth,
		int area,
		DirectX::XMFLOAT4X4 lvp
	);
	DirectX::XMFLOAT4X4 CalcTexelSnappedCrop(
		CameraBase& camera,
		float depth,
		int area,
		float res,
		DirectX::XMFLOAT4X4 lv
	);

public:

	void SetCascadeAreas(UINT idx, float value) { assert(idx < ShadowParam::CASCADE_COUNT); CascadeAreas[idx] = value; };

	/// <summary>
	/// 描画中のカスケード番号（シャドウマップ用マテリアルがBindで参照する）
	/// </summary>
	UINT GetCurrentCascade() const { return CurrentCascade; }

private:

	float CascadeAreas[ShadowParam::CASCADE_COUNT];
	UINT CurrentCascade = 0;
	ShadowParam::ShadowMapsParam ShadowMapsParam;
	ShadowParam::ShadowReceieverParam ShadowReceiveParam;

private:

	// テクスチャ
	std::vector<std::unique_ptr<DepthStencil>> DSVs;
	std::vector<std::shared_ptr<RenderTarget>> StagingShadowMaps;
	std::vector<std::shared_ptr<RenderTarget>> StagingVSMShadowMaps;
	// シェーダーから参照するSRV（ステージングのSRVをコピーしたもの）
	std::vector<DescriptorHeap::Handle> ShadowMapSRVs;
	std::vector<DescriptorHeap::Handle> VSMShadowMapSRVs;

	// ガウスパラメータ
	UINT32 GaussIdx[ShadowParam::CASCADE_COUNT];

};

#endif
