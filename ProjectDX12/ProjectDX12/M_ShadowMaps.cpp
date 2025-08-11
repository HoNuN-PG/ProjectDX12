
#include "M_ShadowMaps.h"
#include "GlobalResourceKey.h"
#include "RenderingEngine.h"

#include "GameObject.h"

UINT M_ShadowMapsBase::CurrentShadowMapsNo = 0;

void M_SimpleShadowMaps::Initialize(DescriptorHeap* heap)
{
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = heap;
		// Params
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // ライト
		desc.size = sizeof(ShadowParam::ShadowMapsParam);
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // シャドウマップ1
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // シャドウマップ2
		Params.push_back(std::make_unique<ConstantBuffer>(desc)); // シャドウマップ3
	}

	RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_VERTEX},
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
	pipeline.VSFile = L"assets/shader/VS_ShadowMap.cso";
	pipeline.PSFile = L"assets/shader/PS_SimpleShadowMap.cso";
	pipeline.RenderTargetNum = 1;

	Material::Create
	(
		heap,
		rootsignature,
		pipeline
	);
}

void M_SimpleShadowMaps::Bind()
{
	// 定数バッファの設定
	WriteParams((UINT)1, 0,
		RenderingEngine::GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::Light).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	WriteParams((UINT)1, 1 + CurrentShadowMapsNo,
		RenderingEngine::GetGlobalConstantBufferResource(GlobalConstantBufferResourceKey::ShadowMaps1 + CurrentShadowMapsNo).hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE desc[] =
	{
		WVP[MaterialInstanceIdx]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
		Params[1 + CurrentShadowMapsNo]->GetHandle().hGPU,
	};
	Material::BindBase(desc, _countof(desc));
}
