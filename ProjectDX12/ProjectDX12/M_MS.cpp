#include "M_MS.h"

void M_MS::Initialize(Description desc)
{
	// ルートシグネチャ
	RootSignature::Parameter param[] =
	{
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_MESH},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_MESH},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_MESH},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_MESH},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1, D3D12_SHADER_VISIBILITY_MESH},
	};
	RootSignature::Description rootsignature;
	rootsignature.pParam = param;
	rootsignature.paramNum = _countof(param);
	rootsignature.bMeshShader = TRUE;
	CalcMeshShaderSRVStartSlot(param, _countof(param));

	// パイプライン
	Pipeline::Description pipeline;
	pipeline.MeshShader = TRUE;
	pipeline.MSFile = L"../game/assets/shader/MS_Object.cso";
	pipeline.PSFile = L"../game/assets/shader/PS_MS.cso";
	pipeline.CullMode = desc.CullMode;
	pipeline.RenderTargetNum = 1;
	pipeline.WriteDepth = desc.WriteDepth;

	Material::SetUp(
		desc.pHeap,
		rootsignature,
		pipeline
	);
}

void M_MS::Bind(UINT materialinstance)
{
	RootSignature::CustomBindSetting setting[] =
	{
		{WVP[materialinstance]->GetHandle().hGPU, TRUE },
		{D3D12_GPU_DESCRIPTOR_HANDLE()			, FALSE},
		{D3D12_GPU_DESCRIPTOR_HANDLE()			, FALSE},
		{D3D12_GPU_DESCRIPTOR_HANDLE()			, FALSE},
		{D3D12_GPU_DESCRIPTOR_HANDLE()			, FALSE},
	};
	Material::BindBase(setting, _countof(setting));
}

void M_MS::CalcMeshShaderSRVStartSlot(RootSignature::Parameter* parameters, UINT num)
{
	// メッシュシェーダーのSRVは順番にまとまっている想定
	// 1.入力 2.Meshlet 3.頂点インデックス 4.プリミティブインデックス
	for (int i = 0; i < num; ++i)
	{
		if(parameters[i].type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV && parameters[i].shader == D3D12_SHADER_VISIBILITY_MESH)
		{
			MeshShaderSRVStartSlot = i;
			return;
		}
	}
}
