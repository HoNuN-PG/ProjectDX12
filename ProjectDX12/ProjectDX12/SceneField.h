#ifndef __SCENE_FIELD_H__
#define __SCENE_FIELD_H__

#include "SceneBase.h"

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"
#include <vector>

#include "CameraBase.h"

class SceneField : public SceneBase
{
public:
	HRESULT Init();
	void Uninit();
	void Update();
	void Draw();

private:
	MeshBuffer* m_pPlane;
	DescriptorHeap* m_pShaderHeap;
	DescriptorHeap* m_pDSVHeap;
	std::vector<ConstantBuffer*> m_pWVPs;
	std::vector<ConstantBuffer*> m_pParams;
	RootSignature* m_pRootSignature;
	RootSignature* m_pWaterRS;
	std::vector<Pipeline*> m_pPipelines;
	DepthStencil* m_pDSV;
	MeshBuffer* m_pSphere;
	RootSignature* m_pSkyRS;

private:
	CameraDebug* m_pCamera;

};

#endif // __SCENE_PROCEDURAL_H__