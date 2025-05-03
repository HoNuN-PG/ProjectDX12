#ifndef __SCENE_SAND_BOX_DX12_H__
#define __SCENE_SAND_BOX_DX12_H__

#include "SceneBase.h"

#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "DepthStencil.h"

#include <vector>
#include <memory>

#include "sphere.h"
#include "Model.h"
#include "Material.h"

class SceneSandBoxDX12 : public SceneBase
{
public:
	HRESULT Init();
	void Uninit();
	void Update();
	void Draw();

private:
	std::unique_ptr<Sphere>							SphereMesh;
	std::unique_ptr<Model>							ModelMesh;
	std::unique_ptr<DescriptorHeap>					Heap;
	std::unique_ptr<DescriptorHeap>					DSVHeap;
	std::vector<std::unique_ptr<ConstantBuffer>>	WVPs;
	std::vector<std::unique_ptr<ConstantBuffer>>	Params;
	std::unique_ptr<RootSignature>					RootSignatureData;
	std::unique_ptr<Pipeline>						PipelineData;
	std::unique_ptr<DepthStencil>					DSV;

private:
	std::vector<std::unique_ptr<Material>>			Materials;

};

#endif // __SCENE_PROCEDURAL_H__