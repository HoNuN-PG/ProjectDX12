
#include "M_Grid.h"
#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

#include "GameObject.h"

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
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // カメラ
	}

	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
	};
	RootSignature::DescriptionTable rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);
	rootsignature.sample = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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
	pipeline.VSFile = L"assets/shader/VS_WorldObjectLocalNormal.cso";
	pipeline.PSFile = L"assets/shader/PS_Grid.cso";
	pipeline.RenderTargetNum = 1;

	Material::Create
	(
		heap,
		rootsignature,
		pipeline
	);

	AddTexture("assets/texture/Mask/grid.png");
}

void M_Grid::Bind()
{
	// 定数バッファの設定
	DirectX::XMFLOAT3 posWS = Owner.lock()->GetWorldPosition();
	Grid.ObjectPositionWS = {
		posWS.x,
		posWS.y,
		posWS.z,
		1
	};
	WriteParams(&Grid,0);
	WriteParams((UINT)1, 1,
		RenderingEngine::GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Camera).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Textures[0]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
