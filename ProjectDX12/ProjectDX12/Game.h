#ifndef ___GAME_H___
#define ___GAME_H___

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

class cGame
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
	static HRESULT InitGame();
	static void UninitGame();
	static void UpdateGame();
	static void DrawGame();
private:
	static std::unique_ptr<cCameraDebug>				m_pCamera;
	static std::unique_ptr<cLightBase>					m_pLight;
private:
	static std::shared_ptr<MeshBuffer>					m_pScreen;
	static std::vector<std::shared_ptr<MeshBuffer>>		m_pModel;
	static std::shared_ptr<ConstantBuffer>				m_pObjectCB_WVP[MAX_WVP];
	static std::shared_ptr<ConstantBuffer>				m_pObjectCB_WVP_Gbuffer[3];
	static std::shared_ptr<ConstantBuffer>				m_pObjectCB_IBL;
	static std::shared_ptr<ConstantBuffer>				m_pObjectCB_Camera;
	static std::shared_ptr<RootSignature>				m_pRootSignature;
	static std::shared_ptr<RootSignature>				m_pRootSignatureDeffered;
	static std::shared_ptr<Pipeline>					m_pPipeline;
	static std::shared_ptr<Pipeline>					m_pSkyPipeline;
	static std::shared_ptr<Pipeline>					m_pBlurPipeline;
	static std::shared_ptr<Pipeline>					m_pPipelineDefferedWrite;
	static std::shared_ptr<Pipeline>					m_pPipelineDeffered;
	static std::shared_ptr<cTexture>					m_pTexture;
	static std::shared_ptr<cRenderTarget>				m_pRTV[4];
	static std::shared_ptr<DescriptorHeap>				m_pHeap;
	static std::shared_ptr<DescriptorHeap>				m_pRTVHeap;
	static std::shared_ptr<DescriptorHeap>				m_pDSVHeap;
	static std::shared_ptr<DepthStencil>				m_pDSV;
	static std::shared_ptr<MeshBuffer>					m_pSkySphere;
	static std::shared_ptr<cTexture>					m_pHDRI;
};

#endif