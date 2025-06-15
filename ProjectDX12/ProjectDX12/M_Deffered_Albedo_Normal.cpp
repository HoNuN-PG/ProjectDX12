
#include "M_Deffered_Albedo_Normal.h"

void M_Deffered_Albedo_Normal::Initialize(DescriptorHeap* heap, RenderingTiming timing)
{
	Timing = timing;

	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// WVP
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		WVP = std::make_unique<ConstantBuffer>(desc);
	}

	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
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
	pipeline.VSFile = L"assets/shader/VS_Object.cso";
	pipeline.PSFile = L"assets/shader/PS_Deffered.cso";
	pipeline.RenderTargetNum = 2;

	Material::Create
	(
		heap,
		param,
		_countof(param),
		pipeline
	);
}

void M_Deffered_Albedo_Normal::Draw()
{
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP->GetHandle().hGPU,
		Textures[0]->GetHandle().hGPU,
	};
	Material::DrawBase(desc, _countof(desc));
}
