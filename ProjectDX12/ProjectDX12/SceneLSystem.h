#ifndef __SCENE_LSYSTEM_H__
#define __SCENE_LSYSTEM_H__

#include "SceneBase.h"

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"

#include "CameraBase.h"

class SceneLSystem : public SceneBase
{
public:
	HRESULT Init();
	void Uninit();
	void Update();
	void Draw();

private:
	MeshBuffer* m_pTree;
	DescriptorHeap* m_pShaderHeap;
	DescriptorHeap* m_pDSVHeap;
	ConstantBuffer* m_pWVP;
	RootSignature* m_pRootSignature;
	Pipeline* m_pPipeline;
	DepthStencil* m_pDSV;

private:
	CameraDebug* m_pCamera;

};

#endif // __SCENE_LSYSTEM_H__