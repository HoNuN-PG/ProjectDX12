
#include "Material.h"

#include "SceneManager.h"
#include "RenderingEngine.h"

Material::Material()
{
	MaterialInstanceCount = 0;
	MaterialInstanceIdx = 0;
}

void Material::Initialize(
	std::shared_ptr<Material> material, 
	DescriptorHeap* heap, 
	RenderingTiming timing, 
	RenderingPass::RenderingPassType passType)
{
	material->Timing = timing;
	material->PassType = passType;
	material->Initialize(heap);
	SceneManager::GetCurrentScene()->GetRenderingEngine()->AddRenderingMaterial(material);
}

void Material::Create(
	DescriptorHeap* heap, 
	RootSignature::DescriptionTable rootsignature,
	Pipeline::Description pipeline)
{
	Heap = heap;

	// ルートシグネチャ
	{
		RootSignatureData = std::make_unique<RootSignature>(rootsignature);
	}
	// パイプライン
	{
		Pipeline::Description desc = {};
		desc.cull = pipeline.cull;
		desc.pInputLayout = pipeline.pInputLayout;
		desc.InputLayoutNum = pipeline.InputLayoutNum;
		desc.VSFile = pipeline.VSFile;
		desc.PSFile = pipeline.PSFile;
		desc.pRootSignature = RootSignatureData->Get();
		desc.RenderTargetNum = pipeline.RenderTargetNum;
		PipelineData = std::make_unique<Pipeline>(desc);
	}
}

void Material::BindBase(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT handleNum)
{
	PipelineData->Bind();
	ID3D12DescriptorHeap* heaps[] =
	{
		Heap->Get(),
	};
	DescriptorHeap::Bind(heaps,1);
	RootSignatureData->Bind(handle, handleNum);
	MaterialInstanceIdx++;
	if (MaterialInstanceIdx >= MaterialInstanceCount)
	{ // MaterialInstanceIdxがループするように対応
		MaterialInstanceIdx = 0;
	}
}

void Material::AddTexture(const char* path)
{
	Texture::Description desc = {};
	desc.fileName = path;
	desc.pHeap = Heap;
	Textures.push_back(std::make_unique<Texture>(desc));
}

void Material::AddMaterialInstance()
{
	MaterialInstanceCount++;
	ConstantBuffer::Description desc = {};
	desc.pHeap = Heap;
	// WVP
	desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
	WVP.push_back(std::make_unique<ConstantBuffer>(desc));
}

void Material::WriteWVP(void* data)
{
	if (MaterialInstanceIdx >= MaterialInstanceCount) {
		return;
	}
	WVP[MaterialInstanceIdx]->Write(data);
}

void Material::WriteParams(void* data, UINT idx)
{
	Params[idx]->Write(data);
}

void Material::WriteParams(UINT range, UINT startIdx, D3D12_CPU_DESCRIPTOR_HANDLE startHandle, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	GetDevice()->CopyDescriptorsSimple(
		range,
		Params[startIdx]->GetHandle().hCPU,
		startHandle,
		type);
}

void Material::EndRendering()
{
	MaterialInstanceIdx = 0;
}
