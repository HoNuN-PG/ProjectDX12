
// Material
#include "Material.h"

// Scene
#include "SceneManager.h"

// System/Rendering
#include "RenderingEngine.h"

Material::Material()
{
	MaterialInstanceCount = 0;
	MaterialInstanceIdx = 0;
}

void Material::Initialize(std::shared_ptr<Material> material, Description desc)
{
	material->Timing = desc.Timing;
	material->PassType = desc.PassType;
	material->Initialize(desc);

	// レンダリングエンジンにマテリアルの参照を登録
	SceneManager::GetCurrentScene()->GetRenderingEngine()->RegisterMaterial(material);
}

void Material::SetUp(
	DescriptorHeap* heap, 
	RootSignature::Description rootsignature,
	Pipeline::Description pipeline,
	UINT rtvNum
)
{
	Heap = heap;

	// ルートシグネチャ
	{
		RootSignatureData = std::make_unique<RootSignature>(rootsignature);
	}
	// パイプライン
	{
		Pipeline::Description desc = {};
		desc.pRootSignature = RootSignatureData->Get();
		desc.VSFile = pipeline.VSFile;
		desc.PSFile = pipeline.PSFile;
		desc.pInputLayout = pipeline.pInputLayout;
		desc.InputLayoutNum = pipeline.InputLayoutNum;
		desc.RenderTargetNum = pipeline.RenderTargetNum;
		desc.CullMode = pipeline.CullMode;
		desc.WriteDepth = pipeline.WriteDepth;
		PipelineData = std::make_unique<Pipeline>(desc);
	}
	// マテリアルディスクリプターヒープ(レンダーターゲット)
	if(rtvNum > 0)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num = rtvNum;
		RTVHeap = std::make_shared<DescriptorHeap>(desc);
	}
}

void Material::BindBase(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT handleNum)
{
	ID3D12DescriptorHeap* heaps[] =
	{
		Heap->Get(),
	};
	DescriptorHeap::Bind(heaps,1);
	RootSignatureData->Bind(handle, handleNum);
	PipelineData->Bind();

	// マテリアルインデックス更新
	MaterialInstanceIdx = max(++MaterialInstanceIdx, MaterialInstanceCount);
}

void Material::AddMaterialInstance()
{
	// マテリアルインデックス追加
	++MaterialInstanceCount;

	// WVP確保
	ConstantBuffer::Description desc = {};
	desc.pHeap = Heap;
	desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
	WVP.push_back(std::make_unique<ConstantBuffer>(desc));
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
	// WVP書き込み
	if (MaterialInstanceIdx >= MaterialInstanceCount) return;
	WVP[MaterialInstanceIdx]->Write(data);
}

void Material::WriteParam(void* data, UINT idx)
{
	Params[idx]->Write(data);
}

void Material::WriteParams(UINT range, UINT startIdx, D3D12_CPU_DESCRIPTOR_HANDLE startHandle, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	GetDevice()->CopyDescriptorsSimple(
		range,
		Params[startIdx]->GetHandle().hCPU,
		startHandle,
		type
	);
}

void Material::Refresh()
{
	MaterialInstanceIdx = 0;
}
