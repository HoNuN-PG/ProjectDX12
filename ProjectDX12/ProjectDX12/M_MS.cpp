
// Game/Camera
#include "CameraBase.h"

// Material/Materials
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
	CalcMeshShaderStartSlot_SRV(param, _countof(param));

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

void M_MS::CalcMeshShaderStartSlot_SRV(RootSignature::Parameter* parameters, UINT num)
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

void M_MSCulling::Initialize(Description desc)
{
	// 定数バッファ
	{
		ConstantBuffer::Description constant = {};
		constant.pHeap = desc.pHeap;
		// Params
		constant.size = sizeof(DirectX::XMFLOAT4) * 6;
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // フラスタム
		constant.size = sizeof(M_MSCulling::MESHLET_INFO);
		Params.push_back(std::make_unique<ConstantBuffer>(constant)); // メッシュレットカウント
	}

	// ルートシグネチャ
	RootSignature::Parameter param[] =
	{
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_AMPLIFICATION},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_AMPLIFICATION},
		{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_AMPLIFICATION},
		{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_AMPLIFICATION},
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
	CalcAmpShaderStartSlot_SRV(param, _countof(param));
	CalcMeshShaderStartSlot_SRV(param, _countof(param));

	// パイプライン
	Pipeline::Description pipeline;
	pipeline.AmpShader = TRUE;
	pipeline.MeshShader = TRUE;
	pipeline.ASFile = L"../game/assets/shader/AS_CullingObject.cso";
	pipeline.MSFile = L"../game/assets/shader/MS_CullingObject.cso";
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

void M_MSCulling::Bind(UINT materialinstance)
{
	std::vector<DirectX::XMFLOAT4> planes = CameraBase::GetMainFrustumPlanes();
	Params[0]->Write(planes.data());

	RootSignature::CustomBindSetting setting[] =
	{
		{WVP[materialinstance]->GetHandle().hGPU, TRUE },
		{Params[0]->GetHandle().hGPU			, TRUE },
		{Params[1]->GetHandle().hGPU			, TRUE },
		{D3D12_GPU_DESCRIPTOR_HANDLE()			, FALSE},
		{WVP[materialinstance]->GetHandle().hGPU, TRUE },
		{D3D12_GPU_DESCRIPTOR_HANDLE()			, FALSE},
		{D3D12_GPU_DESCRIPTOR_HANDLE()			, FALSE},
		{D3D12_GPU_DESCRIPTOR_HANDLE()			, FALSE},
		{D3D12_GPU_DESCRIPTOR_HANDLE()			, FALSE},
	};
	Material::BindBase(setting, _countof(setting));
}

void M_MSCulling::WriteMeshletCount(int count)
{
	M_MSCulling::MESHLET_INFO info;
	info.meshletCount = count;
	Params[1]->Write(&info);
}

void M_MSCulling::CalcAmpShaderStartSlot_SRV(RootSignature::Parameter* parameters, UINT num)
{
	// 増幅シェーダーのSRVは順番にまとまっている想定
	// 1.CullData
	for (int i = 0; i < num; ++i)
	{
		if (parameters[i].type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV && parameters[i].shader == D3D12_SHADER_VISIBILITY_AMPLIFICATION)
		{
			AmpShaderSRVStartSlot = i;
			return;
		}
	}
}

void M_MSCulling::CalcMeshShaderStartSlot_SRV(RootSignature::Parameter* parameters, UINT num)
{
	// メッシュシェーダーのSRVは順番にまとまっている想定
	// 1.入力 2.Meshlet 3.頂点インデックス 4.プリミティブインデックス
	for (int i = 0; i < num; ++i)
	{
		if (parameters[i].type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV && parameters[i].shader == D3D12_SHADER_VISIBILITY_MESH)
		{
			MeshShaderSRVStartSlot = i;
			return;
		}
	}
}
