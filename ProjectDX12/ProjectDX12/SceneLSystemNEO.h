#ifndef __SCENE_LSYSTEM_NEO_H__
#define __SCENE_LSYSTEM_NEO_H__

#include "SceneBase.h"

#include <vector>

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"

#include "CameraBase.h"
#include "LightBase.h"

class SceneLSystemNEO : public SceneBase
{
public:
	HRESULT Init();
	void Uninit();
	void Update();
	void Draw();
private:
	std::vector<MeshBuffer*> m_pTree;
	DescriptorHeap* m_pShaderHeap;
	DescriptorHeap* m_pDSVHeap;
	std::vector<DirectX::XMMATRIX> m_W;
	std::vector<ConstantBuffer*> m_pWVP;
	std::vector<ConstantBuffer*> m_pLightBuffer;
	RootSignature* m_pRootSignature;
	Pipeline* m_pPipeline;
	DepthStencil* m_pDSV;
private:
	cCameraDebug* m_pCamera;
	cLightBase* m_pLight;
};

#endif // __SCENE_LSYSTEM_NEO_H__