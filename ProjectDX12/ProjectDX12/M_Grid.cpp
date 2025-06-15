#include "M_Grid.h"

void M_Grid::Initialize(DescriptorHeap* heap, RenderingTiming timing)
{
	Timing = timing;

	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// Params
		desc.size = sizeof(GridParam);
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
	}

	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
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
	pipeline.VSFile = L"assets/shader/VS_WorldObject.cso";
	pipeline.PSFile = L"assets/shader/PS_Grid.cso";
	pipeline.RenderTargetNum = 1;

	Material::Create
	(
		heap,
		param,
		_countof(param),
		pipeline
	);
}

void M_Grid::Bind()
{
	// 定数バッファの設定
	WriteParams(&Grid,0);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
