
#include "GlobalResourceKey.h"

#include "M_SimpleLit.h"
#include "RenderingEngine.h"

void M_SimpleLit::Initialize(DescriptorHeap* heap, RenderingTiming timing)
{
	Timing = timing;

	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// WVP
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		WVP = std::make_unique<ConstantBuffer>(desc);
		// Params
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // カメラ
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // ライト
	}

	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
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
	pipeline.PSFile = L"assets/shader/PS_SimpleLit.cso";
	pipeline.RenderTargetNum = 1;

	Material::Create
	(
		heap,
		param,
		_countof(param),
		pipeline
	);
}

void M_SimpleLit::Draw()
{
	// 定数バッファの設定
	WriteParams((UINT)2, 0,
		RenderingEngine::GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
	};
	Material::DrawBase(desc, _countof(desc));
}
