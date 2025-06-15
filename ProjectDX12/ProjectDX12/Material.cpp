
#include "Material.h"

void Material::Create(
	DescriptorHeap* heap, 
	RootSignature::ParameterTable* param, 
	UINT paranNum, 
	Pipeline::Description pipeline)
{
	Heap = heap;

	// ルートシグネチャ
	{
		RootSignature::DescriptionTable desc = {};
		desc.pParam = param;
		desc.paramNum = paranNum;
		RootSignatureData = std::make_unique<RootSignature>(desc);
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

void Material::DrawBase(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT handleNum)
{
	PipelineData->Bind();
	ID3D12DescriptorHeap* heaps[] =
	{
		Heap->Get(),
	};
	DescriptorHeap::Bind(heaps,1);
	RootSignatureData->Bind(handle, handleNum);
}

void Material::AddTexture(const char* path)
{
	Texture::Description desc = {};
	desc.fileName = path;
	desc.pHeap = Heap;
	Textures.push_back(std::make_unique<Texture>(desc));
}

void Material::WriteWVP(void* data)
{
	WVP->Write(data);
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