
#include "M_DepthNormal.h"

void M_DepthNormal::Initialize(DescriptorHeap* heap)
{
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
	}

	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
	};
	RootSignature::DescriptionTable rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);
	Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"NORMAL",   0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
			{"COLOR",    0,DXGI_FORMAT_R32G32B32A32_FLOAT},
	};
	Pipeline::Description pipeline;
	pipeline.cull = D3D12_CULL_MODE_BACK;
	pipeline.pInputLayout = layout;
	pipeline.InputLayoutNum = _countof(layout);
	pipeline.VSFile = L"assets/shader/VS_DepthNormal.cso";
	pipeline.PSFile = L"assets/shader/PS_DepthNormal.cso";
	pipeline.RenderTargetNum = 2;

	Material::Create
	(
		heap,
		rootsignature,
		pipeline
	);
}

void M_DepthNormal::Bind()
{
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
