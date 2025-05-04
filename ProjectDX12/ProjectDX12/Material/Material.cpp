
#include "Material.h"

void Material::Create
(
	DescriptorHeap* heap, 
	RootSignature::ParameterTable* param, 
	UINT paranNum, 
	Pipeline::InputLayout* layout, 
	UINT layoutNum, 
	const wchar_t* vsPath, 
	const wchar_t* psPath, 
	UINT rtNum)
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
		desc.pInputLayout = layout;
		desc.InputLayoutNum = layoutNum;
		desc.VSFile = vsPath;
		desc.PSFile = psPath;
		desc.pRootSignature = RootSignatureData->Get();
		desc.RenderTargetNum = rtNum;
		PipelineData = std::make_unique<Pipeline>(desc);
	}
}

void Material::Create
(
	DescriptorHeap* heap,
	RootSignature::ParameterTables* param, 
	UINT paranNum,
	Pipeline::InputLayout* layout, 
	UINT layoutNum,
	const wchar_t* vsPath, 
	const wchar_t* psPath,
	UINT rtNum
)
{
	Heap = heap;

	// ルートシグネチャ
	{
		RootSignature::DescriptionTables desc	= {};
		desc.pParam								= param;
		desc.paramNum							= paranNum;
		RootSignatureData						= std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		Pipeline::Description desc	= {};
		desc.pInputLayout			= layout;
		desc.InputLayoutNum			= layoutNum;
		desc.VSFile					= vsPath;
		desc.PSFile					= psPath;
		desc.pRootSignature			= RootSignatureData->Get();
		desc.RenderTargetNum		= rtNum;
		PipelineData				= std::make_unique<Pipeline>(desc);
	}
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
		Params[startIdx]->GetHandle().hCPU, startHandle,
		type);
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
