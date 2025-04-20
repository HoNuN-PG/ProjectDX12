#ifndef ___SCENE_IBL_H___
#define ___SCENE_IBL_H___

#include "SceneBase.h"

#include "DirectX.h"
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")
#include <DirectXMath.h>

#include <memory>
#include <vector>

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "DepthStencil.h"

#include "CameraBase.h"
#include "LightBase.h"

enum WVP_TYPE
{
	MODEL = 0,
	SKY_SPHERE,
	BLUR,
	DEFFERED,

	MAX_WVP
};

class SceneIBL : public SceneBase
{
public:
	struct Vertex {
		float pos[3];
		float uv[2];
		float color[4];
	};
	struct ModelVertex {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
	};
public:
	HRESULT Init();
	void Uninit();
	void Update();
	void Draw();
private:
	std::unique_ptr<cCameraDebug>				m_pCamera;
	std::unique_ptr<cLightBase>					m_pLight;
private:
	std::shared_ptr<MeshBuffer>					m_pScreen;
	std::vector<std::shared_ptr<MeshBuffer>>		m_pModel;
	std::shared_ptr<ConstantBuffer>				m_pObjectCB_WVP[MAX_WVP];
	std::shared_ptr<ConstantBuffer>				m_pObjectCB_WVP_Gbuffer[3];
	std::shared_ptr<ConstantBuffer>				m_pObjectCB_IBL;
	std::shared_ptr<ConstantBuffer>				m_pObjectCB_Camera;
	std::shared_ptr<RootSignature>				m_pRootSignature;
	std::shared_ptr<RootSignature>				m_pRootSignatureDeffered;
	std::shared_ptr<Pipeline>					m_pPipeline;
	std::shared_ptr<Pipeline>					m_pSkyPipeline;
	std::shared_ptr<Pipeline>					m_pBlurPipeline;
	std::shared_ptr<Pipeline>					m_pPipelineDefferedWrite;
	std::shared_ptr<Pipeline>					m_pPipelineDeffered;
	std::shared_ptr<cTexture>					m_pTexture;
	std::shared_ptr<RenderTarget>				m_pRTV[4];
	std::shared_ptr<DescriptorHeap>				m_pHeap;
	std::shared_ptr<DescriptorHeap>				m_pRTVHeap;
	std::shared_ptr<DescriptorHeap>				m_pDSVHeap;
	std::shared_ptr<DepthStencil>				m_pDSV;
	std::shared_ptr<MeshBuffer>					m_pSkySphere;
	std::shared_ptr<cTexture>					m_pHDRI;
};

#endif