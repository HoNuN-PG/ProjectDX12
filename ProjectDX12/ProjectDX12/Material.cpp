
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

void Material::Initialize(
	std::shared_ptr<Material> material, 
	DescriptorHeap* heap, 
	Description desc,
	RenderingTiming timing, 
	RenderingPass::RenderingPassType passType
)
{
	material->Timing = timing;
	material->PassType = passType;
	material->Initialize(heap,desc);

	// レンダリングエンジンにマテリアルを登録
	SceneManager::GetCurrentScene()->GetRenderingEngine()->AddRenderingMaterial(material);
}

void Material::SetUp(
	DescriptorHeap* heap, 
	RootSignature::DescriptionTable rootsignature,
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
		desc.cull = pipeline.cull;
		desc.WriteDepth = pipeline.WriteDepth;
		desc.pInputLayout = pipeline.pInputLayout;
		desc.InputLayoutNum = pipeline.InputLayoutNum;
		desc.VSFile = pipeline.VSFile;
		desc.PSFile = pipeline.PSFile;
		desc.pRootSignature = RootSignatureData->Get();
		desc.RenderTargetNum = pipeline.RenderTargetNum;
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
	PipelineData->Bind();
	ID3D12DescriptorHeap* heaps[] =
	{
		Heap->Get(),
	};
	DescriptorHeap::Bind(heaps,1);
	RootSignatureData->Bind(handle, handleNum);

	// マテリアルインデックス更新
	MaterialInstanceIdx = (MaterialInstanceIdx + 1) % MaterialInstanceCount;
}

void Material::AddMaterialInstance()
{
	// マテリアルインデックス追加
	MaterialInstanceCount++;

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
		type);
}

void Material::RefreshRendering()
{
	MaterialInstanceIdx = 0;
}
