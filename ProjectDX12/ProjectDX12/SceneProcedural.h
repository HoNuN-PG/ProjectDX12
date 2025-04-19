#ifndef __SCENE_PROCEDURAL_H__
#define __SCENE_PROCEDURAL_H__

#include "SceneBase.h"

#include "MeshBuffer.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"
#include <vector>

class SceneProcedural : public SceneBase
{
public:
	HRESULT Init();
	void Uninit();
	void Update();
	void Draw();

private:
	MeshBuffer* m_pSprite;
	DescriptorHeap* m_pShaderHeap;
	DescriptorHeap* m_pDSVHeap;
	std::vector<ConstantBuffer*> m_pWVPs;
	std::vector<ConstantBuffer*> m_pParams;
	RootSignature* m_pRootSignature;
	std::vector<Pipeline*> m_pPipelines;
	DepthStencil* m_pDSV;
};

#endif // __SCENE_PROCEDURAL_H__